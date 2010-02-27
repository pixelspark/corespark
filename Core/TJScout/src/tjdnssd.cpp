#include "../include/tjdnssd.h"

#ifdef TJ_OS_WIN
	#include <mDNS/include/dns_sd.h>
#endif

#ifdef TJ_OS_POSIX
	#include <dns_sd.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netdb.h>
	#define SOCKET_ERROR -1
	#define TIMEVAL struct timeval
#endif

using namespace tj::shared;
using namespace tj::scout;
using namespace tj::np;

ref<DNSSDBrowserThread> DNSSDBrowserThread::_instance;

/** DNSSDRequest **/
DNSSDRequest::DNSSDRequest(DNSServiceRef service, bool oneShot): _service(service), _oneShot(oneShot) {
}

DNSSDRequest::~DNSSDRequest() {
	DNSServiceRefDeallocate(_service);
}

void DNSSDRequest::OnReceive(NativeSocket ns) {
	DNSServiceProcessResult(_service);
	if(_oneShot) {
		ref<DNSSDBrowserThread> bt = _thread;
		if(bt) {
			bt->RemoveRequest(ref<DNSSDRequest>(this));
		}
	}
}

/** DNSSDBrowserThread **/
DNSSDBrowserThread::DNSSDBrowserThread() {
}

DNSSDBrowserThread::~DNSSDBrowserThread() {
}

void DNSSDBrowserThread::AddRequest(ref<DNSSDRequest> drq) {
	ThreadLock lock(&_lock);
	if(drq) {
		ref<DNSSDBrowserThread> sr= this;
		drq->_thread = sr;
		_requests.insert(drq);
		AddListener(DNSServiceRefSockFD(drq->_service), drq);
	}
}

void DNSSDBrowserThread::RemoveRequest(ref<DNSSDRequest> drq) {
	ThreadLock lock(&_lock);
	if(drq) {
		ref<DNSSDBrowserThread> tr;
		drq->_thread = tr;
		std::set< ref<DNSSDRequest> >::iterator it = _requests.find(drq);
		if(it!=_requests.end()) {
			_requests.erase(it);
		}
		RemoveListener(DNSServiceRefSockFD(drq->_service));
	}
}

strong<DNSSDBrowserThread> DNSSDBrowserThread::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new DNSSDBrowserThread());
		_instance->Start();
	}
	return _instance;
}

/** DNSSDResolveRequest **/
void DNSSDDiscoveryBrowseReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regType, const char *replyDomain, void *context)  {
	try {
		if(context!=0) {
			ref<DNSSDResolveRequest> rr = ref<DNSSDResolveRequest>((DNSSDResolveRequest*)context);
			if(rr) {
				ref<ResolveRequest> req = rr->GetOriginalRequest();
				if(req) {
					ref<Service> service = GC::Hold(new DNSSDService(Wcs(serviceName), Wcs(regType), Wcs(replyDomain), interfaceIndex));

					if((flags & kDNSServiceFlagsAdd)!=0) {
						req->OnServiceFound(service);
					}
					else {
						req->OnServiceDisappeared(service->GetID());
					}
				}
			}
		}
	}
	catch(Exception& e) {
		Log::Write(L"TJScout/DNSSDBrowser", L"Exception occurred when a new service was found: "+e.GetMsg());
	}
	catch(...) {
		Log::Write(L"TJScout/DNSSDBrowser", L"Unknown exception occurred when a new service was found");
	}
}

DNSSDResolveRequest::DNSSDResolveRequest(ref<ResolveRequest> rq, const std::wstring& type): DNSSDRequest(NULL,false), _request(rq), _type(type) {
}

void DNSSDResolveRequest::OnCreated() {
	RequestResolver::OnCreated();
	std::string serviceType = Mbs(_type);
	DNSServiceErrorType er = DNSServiceBrowse(&(DNSSDRequest::_service), 0, 0, serviceType.c_str(), NULL, (DNSServiceBrowseReply)DNSSDDiscoveryBrowseReply, (void*)this);
	if(er!=kDNSServiceErr_NoError) {
		Log::Write(L"TJScout/DNSSDResolveRequest", L"Could not start browsing for services; err="+Stringify(er)+L"; probably, Bonjour is not installed on this system");
	}
	else {
		DNSSDBrowserThread::Instance()->AddRequest(this);
	}
}

DNSSDResolveRequest::~DNSSDResolveRequest() {
}

ref<ResolveRequest> DNSSDResolveRequest::GetOriginalRequest() {
	return _request;
}

/** DNSSDResolver **/
DNSSDResolver::DNSSDResolver() {
}

DNSSDResolver::~DNSSDResolver() {
}

ref<RequestResolver> DNSSDResolver::Resolve(strong<ResolveRequest> rr) {
	const ServiceDescription& sd = rr->GetDesiredServiceType();
	std::wstring data;
	if(sd.GetDescriptionOfType(ServiceDiscoveryDNSSD,data)) {
		return GC::Hold(new DNSSDResolveRequest(rr,data));
	}
	return null;
}

/** DNSSDAddressFuture **/
DNSSDAddressFuture::DNSSDAddressFuture(unsigned int iface, const char* name, const char* regtype, const char* domain): _service(0) {
	_ri._succeeded = false;
	DNSServiceResolve(&_service, 0, iface, name, regtype, domain, (DNSServiceResolveReply)Reply, reinterpret_cast<void*>(&_ri));
}

DNSSDAddressFuture::~DNSSDAddressFuture() {
}

std::wstring DNSSDAddressFuture::GetAddress() {
	ThreadLock lock(&_lock);
	if(!IsRun() || DidFail() || !_ri._succeeded) {
		Throw(L"Could not resolve the address of the service", ExceptionTypeError);
	}
	return _ri._ip;
}

std::wstring DNSSDAddressFuture::GetHostName() {
	ThreadLock lock(&_lock);
	if(!IsRun() || DidFail() || !_ri._succeeded) {
		Throw(L"Could not resolve the host name of the service", ExceptionTypeError);
	}
	return _ri._hostname;
}

unsigned short DNSSDAddressFuture::GetPort() {
	ThreadLock lock(&_lock);
	if(!IsRun() || DidFail() || !_ri._succeeded) {
		Throw(L"Could not resolve the address of the service; succeeded="+Stringify(_ri._succeeded)+L" run="+Stringify(IsRun())+L" failed="+Stringify(DidFail()), ExceptionTypeError);
	}
	return _ri._port;
}

void DNSSDAddressFuture::Run() {
	DNSServiceProcessResult(_service);
	DNSServiceRefDeallocate(_service);
	_service = 0;
}

void DNSSDAddressFuture::Reply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullname,const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context) {
	ResolvedInfo* ri = reinterpret_cast<ResolvedInfo*>(context);
	if(ri!=0) {
		ri->_succeeded = false;
		hostent* he = gethostbyname(hosttarget);
		if(he!=0) {
			struct in_addr addr;
			addr.s_addr = *(u_long *)he->h_addr_list[0];
			const char* stringAddress = inet_ntoa(addr);
			if(stringAddress!=0) {
				std::string addressMbs = stringAddress;
				ri->_ip = Wcs(addressMbs);
				ri->_hostname = Wcs(hosttarget);
				ri->_port = ntohs(port);
				ri->_succeeded = true;
			}
		}
	}
}

/** DNSSDAttributeResolver **/
class DNSSDAttributeResolver {
	public:
		struct ResolvedInfo {
			ResolvedInfo(std::map<std::wstring, std::wstring>& a): attributes(a), succeeded(false) {
			}

			std::map<std::wstring, std::wstring>& attributes;
			bool succeeded;
		};

		static void Reply(DNSServiceRef DNSServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context) {
			ResolvedInfo* ri = (ResolvedInfo*)context;
			if(ri!=0) {
				ri->succeeded = true;
				const char* rdataChar = (const char*)rdata;
				const unsigned char* rdataUChar = (const unsigned char*)rdata;

				// Parse attribute data
				unsigned int index = 0;
				while(index < rdlen) {
					unsigned int length = rdataUChar[index];
					if(length==0) {
						return;
					}

					unsigned int valueIndex = index;

					while(valueIndex < rdlen) {
						++valueIndex;
						if(rdataChar[valueIndex]=='=') {
							++valueIndex;
							break;
						}
					}
					
					if(valueIndex>index && valueIndex<rdlen && valueIndex < (index+length-1)) {
						std::string key(&(rdataChar[index+1]), valueIndex-index-2);
						std::string value(&rdataChar[valueIndex], length-(valueIndex-index)+1);
						ri->attributes[Wcs(key)] = Wcs(value);
					}
					index += length + 1;
				}
			}
		}

		static bool ResolveAttributesForService(unsigned int iface, const std::wstring& name, std::map<std::wstring,std::wstring>& al) {
			DNSServiceRef service = 0;
			ResolvedInfo ri(al);
			std::string fqdn = Mbs(name);
			DNSServiceQueryRecord(&service, kDNSServiceFlagsLongLivedQuery, iface, fqdn.c_str(), kDNSServiceType_TXT, kDNSServiceClass_IN, (DNSServiceQueryRecordReply)Reply, &ri);
			DNSServiceProcessResult(service);
			DNSServiceRefDeallocate(service);
			return ri.succeeded;
		}
};

/** DNSSDUpdateRequest **/
DNSSDUpdateRequest::DNSSDUpdateRequest(ref<DNSSDService> sd, DNSServiceRef sr): DNSSDRequest(sr,false), _sd(sd) {
}

DNSSDUpdateRequest::~DNSSDUpdateRequest() {
}

void DNSSDUpdateRequest::Reply(DNSServiceRef DNSServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context) {
	ref<DNSSDService> service = ref<DNSSDService>(reinterpret_cast<DNSSDService*>(context));
	if(service) {
		const char* rdataChar = (const char*)rdata;
		const unsigned char* rdataUChar = (const unsigned char*)rdata;
		
		// Parse attribute data
		unsigned int index = 0;
		while(index < rdlen) {
			unsigned int length = rdataUChar[index];
			if(length==0) {
				return;
			}
			
			unsigned int valueIndex = index;
			
			while(valueIndex < rdlen) {
				++valueIndex;
				if(rdataChar[valueIndex]=='=') {
					++valueIndex;
					break;
				}
			}
			
			if(valueIndex>index && valueIndex<rdlen && valueIndex < (index+length-1)) {
				std::string key(&(rdataChar[index+1]), valueIndex-index-2);
				std::string value(&rdataChar[valueIndex], length-(valueIndex-index)+1);
				service->SetAttribute(Wcs(key), Wcs(value));
			}
			index += length + 1;
		}
	}
}


void DNSSDUpdateRequest::OnCreated() {
	DNSSDRequest::OnCreated();
}

strong<DNSSDUpdateRequest> DNSSDUpdateRequest::Create(strong<DNSSDService> sd) {
	DNSServiceRef service;
	std::string fqdn = Mbs(sd->GetQualifiedName());
	if(DNSServiceQueryRecord(&service, 0, sd->GetInterface(), fqdn.c_str(), kDNSServiceType_TXT, kDNSServiceClass_IN, (DNSServiceQueryRecordReply)Reply, reinterpret_cast<void*>(ref<DNSSDService>(sd).GetPointer()))!=kDNSServiceErr_NoError) {
		Throw(L"Cannot start watching updates", ExceptionTypeError);
	}
	return GC::Hold(new DNSSDUpdateRequest(sd, service));
}

/** DNSSDService **/
DNSSDService::DNSSDService(const std::wstring& friendly, const std::wstring& type, const std::wstring& domain, unsigned int iface): _friendly(friendly), _type(type), _domain(domain), _interface(iface) {
	_address = GC::Hold(new DNSSDAddressFuture(iface, Mbs(friendly).c_str(), Mbs(type).c_str(), Mbs(domain).c_str()));
	SharedDispatcher::Instance()->Dispatch(ref<Task>(_address));

	/* Query TXT record containing attributes (a TXT record MUST always be present, hence we can use a blocking query here,
	this also means that non-conforming mDNS responders can block us...) */
	if(!DNSSDAttributeResolver::ResolveAttributesForService(iface, GetQualifiedName(), _attributes)) {
		Log::Write(L"TJScout/DNSSDService", L"Could not resolve attributes for service"+GetID());
	}
}

void DNSSDService::SetAttribute(const String& k, const String& v) {
	ThreadLock lock(&(Service::_lock));
	
	std::map<std::wstring, std::wstring>::iterator it = Service::_attributes.find(k);
	if(it!=_attributes.end()) {
		if(it->second==v) {
			// Value not changed, ignore it
			return;
		}
	}
	
	_attributes[k] = v;
	Service::UpdateNotification un;
	un._attributeChanged = k;
	//Log::Write(L"TJScout/DNSSDService", L"Attribute '"+k+L"' updated to value '"+v+L"' on service "+GetQualifiedName());
	Service::EventUpdate.Fire(this, un);
}

void DNSSDService::OnCreated() {
	Service::OnCreated();
	
	// Start watching TXT record updates asynchronously through a request
	_watcher = DNSSDUpdateRequest::Create(ref<DNSSDService>(this));
	DNSSDBrowserThread::Instance()->AddRequest(_watcher);
}

DNSSDService::~DNSSDService() {
	DNSSDBrowserThread::Instance()->RemoveRequest(_watcher);
}

std::wstring DNSSDService::GetQualifiedName() const {
	return _friendly + L"." + _type + _domain;
}

unsigned int DNSSDService::GetInterface() const {
	return _interface;
}

std::wstring DNSSDService::GetID() const {
	return L"dnssd:" + _friendly + L"." + _type + _domain;
}

std::wstring DNSSDService::GetFriendlyName() const {
	return _friendly;
}

std::wstring DNSSDService::GetType() const {
	return _type;
}

std::wstring DNSSDService::GetAddress() const {
	if(_address && _address->WaitForCompletion()) {
		return _address->GetAddress();
	}
	return L"";
}

std::wstring DNSSDService::GetHostName() const {
	if(_address && _address->WaitForCompletion()) {
		return _address->GetHostName();
	}
	return L"";
}

unsigned short DNSSDService::GetPort() const {
	if(_address && _address->WaitForCompletion()) {
		return _address->GetPort();
	}
	Log::Write(L"TJScout/DNSSDService", L"Wait failed in GetPort");
	return 0;
}

/** DNSSDServiceRegistration **/
DNSSDServiceRegistration::DNSSDServiceRegistration(): ServiceRegistration(ServiceDiscoveryDNSSD) {
	if(DNSServiceCreateConnection(&_sd)!=kDNSServiceErr_NoError) {
		Throw(L"DNSServiceCreateConnection failed; is mDNSResponder installed and running?", ExceptionTypeError);
	}
}

DNSSDServiceRegistration::~DNSSDServiceRegistration() {
	DNSServiceRefDeallocate(_sd);
}

bool DNSSDServiceRegistration::SetAttribute(const String& k, const String& v) {
	_attributes[k] = v;
	
	TXTRecordRef tr;
	unsigned char buffer[2048];
	TXTRecordCreate(&tr, 2048, (void*)buffer);
	
	std::map<std::wstring, std::wstring>::const_iterator it = _attributes.begin();
	while(it!=_attributes.end()) {
		std::string value = Mbs(it->second);
		TXTRecordSetValue(&tr, Mbs(it->first).c_str(), value.length(), value.c_str());
		++it;
	}
	
	if(DNSServiceUpdateRecord(_sd, NULL, 0, TXTRecordGetLength(&tr), TXTRecordGetBytesPtr(&tr), 0)!=kDNSServiceErr_NoError) {
		Log::Write(L"TJScout/DNSSDServiceRegistration", L"Could not update attributes of service after changing key '"+k+L"' to value '"+v+L"'");
		return false;
	}
	return true;
}

void DNSSDServiceRegistration::Register(const ServiceType& st, const std::wstring& name, unsigned short port, const std::map<std::wstring, std::wstring>& attributes) {
	TXTRecordRef tr;
	unsigned char buffer[2048];
	TXTRecordCreate(&tr, 2048, (void*)buffer);
	
	std::map<std::wstring, std::wstring>::const_iterator it = attributes.begin();
	while(it!=attributes.end()) {
		std::string value = Mbs(it->second);
		TXTRecordSetValue(&tr, Mbs(it->first).c_str(), value.length(), value.c_str());
		++it;
	}
	
	DNSServiceRegister(&_sd, 0, 0, Mbs(name).c_str(), Mbs(st).c_str(), NULL, NULL, htons(port), TXTRecordGetLength(&tr), TXTRecordGetBytesPtr(&tr), NULL, NULL);
	TXTRecordDeallocate(&tr);
	_attributes = attributes;
}