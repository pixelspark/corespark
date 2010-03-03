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
DNSSDRequest::DNSSDRequest(DNSServiceRef service, bool oneShot): _service(service), _oneShot(oneShot), _dirty(true) {
}

DNSSDRequest::~DNSSDRequest() {
	if(_dirty) {
		DNSSDBrowserThread::Instance()->RemoveRequest(DNSServiceRefSockFD(_service));
		DNSServiceRefDeallocate(_service);
	}
}

void DNSSDRequest::OnReceive(NativeSocket ns) {
	DNSServiceProcessResult(_service);
	if(_oneShot) {
		ref<DNSSDBrowserThread> bt = _thread;
		if(bt) {
			bt->RemoveRequest(DNSServiceRefSockFD(_service));
			DNSServiceRefDeallocate(_service);
			_dirty = false;
			
		}
	}
}

/** DNSSDBrowserThread **/
DNSSDBrowserThread::DNSSDBrowserThread() {
}

DNSSDBrowserThread::~DNSSDBrowserThread() {
}

void DNSSDBrowserThread::AddRequest(ref<DNSSDRequest> drq) {
	if(drq) {
		ref<DNSSDBrowserThread> sr = this;
		drq->_thread = sr;
		AddListener(DNSServiceRefSockFD(drq->_service), drq);
	}
}

void DNSSDBrowserThread::RemoveRequest(NativeSocket ns) {
	RemoveListener(ns);
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
				if((flags & kDNSServiceFlagsAdd)!=0) {
					ref<DNSSDService> service = GC::Hold(new DNSSDService(Wcs(serviceName), Wcs(regType), Wcs(replyDomain), interfaceIndex));
					rr->AddNewService(service);
				}
				else {
					rr->OnServiceDisappeared(L"dnssd:"+DNSSDService::CreateID(Wcs(serviceName), Wcs(regType), Wcs(replyDomain)));
				}
			}
			else {
				Throw(L"No resolve request given in DNSSDDiscoveryBrowseReply!", ExceptionTypeError);
			}
		}
		else {
			Throw(L"No context given in DNSSDDiscoveryBrowseReply!", ExceptionTypeError);
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
	DNSServiceErrorType er = DNSServiceBrowse(&(DNSSDRequest::_service), 0, 0, serviceType.c_str(), NULL, (DNSServiceBrowseReply)DNSSDDiscoveryBrowseReply, reinterpret_cast<void*>(this));
	if(er!=kDNSServiceErr_NoError) {
		Log::Write(L"TJScout/DNSSDResolveRequest", L"Could not start browsing for services; err="+Stringify(er)+L"; probably, Bonjour is not installed on this system");
	}
	else {
		DNSSDBrowserThread::Instance()->AddRequest(this);
	}
}

DNSSDResolveRequest::~DNSSDResolveRequest() {
}

void DNSSDResolveRequest::Notify(ref<Object> source, const DNSSDService::DNSSDServiceResolvedNotification& data) {
	ref<DNSSDService> service = source;
	if(service) {
		{
			ThreadLock lock(&_lock);
			std::set< ref<DNSSDService> >::iterator it = _waitingToResolve.find(service);
			if(it!=_waitingToResolve.end()) {
				_waitingToResolve.erase(it);
			}
			else {
				Log::Write(L"TJScout/DNSSDResolveRequest", L"Got a resolve notification for a service we we're not waiting for: "+service->GetID());
				return;
			}
		}
		
		ref<ResolveRequest> rr = _request;
		if(rr) {
			rr->OnServiceFound(ref<Service>(service));
		}
	}
}

void DNSSDResolveRequest::OnServiceDisappeared(const String& ident) {
	ref<ResolveRequest> rr = _request;
	if(rr) {
		rr->OnServiceDisappeared(ident);
	}
}

void DNSSDResolveRequest::AddNewService(strong<DNSSDService> dsd) {
	ThreadLock lock(&_lock);
	ref<ResolveRequest> rr = _request;
	if(rr) {
		_waitingToResolve.insert(dsd);
		dsd->EventResolved.AddListener(this);
		dsd->Resolve();
	}
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

/** DNSSDAddressRequest **/
DNSSDAddressRequest::DNSSDAddressRequest(ref<DNSSDService> parent): 
	DNSSDRequest(0, true),
	_succeeded(false),
	_port(0),
	_parent(parent) {

}

DNSSDAddressRequest::~DNSSDAddressRequest() {
}

void DNSSDAddressRequest::OnCreated() {
	ref<DNSSDService> parent = _parent;
	if(parent) {
		std::string name = Mbs(parent->_friendly);
		std::string regType = Mbs(parent->_type);
		std::string domain = Mbs(parent->_domain);
		DNSServiceErrorType er = DNSServiceResolve(&_service, 0, parent->_interface, name.c_str(), regType.c_str(), domain.c_str(), (DNSServiceResolveReply)Reply, reinterpret_cast<void*>(this));
		if(er!=kDNSServiceErr_NoError) {
			Log::Write(L"TJScout/DNSSDAddressRequest", L"Could not resolve address for service; err="+Stringify(er)+L"; probably, Bonjour is not installed on this system");
		}
		else {
			DNSSDBrowserThread::Instance()->AddRequest(this);
		}
	}
}

void DNSSDAddressRequest::Reply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullname,const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context) {
	DNSSDAddressRequest* request = reinterpret_cast<DNSSDAddressRequest*>(context);
	if(request!=0) {
		ref<DNSSDService> service = request->_parent;
		if(service) {
			ThreadLock lock(&(service->_lock));
			request->_succeeded = false;
			hostent* he = gethostbyname(hosttarget);
			if(he!=0) {
				struct in_addr addr;
				addr.s_addr = *(u_long *)he->h_addr_list[0];
				const char* stringAddress = inet_ntoa(addr);
				if(stringAddress!=0) {
					std::string addressMbs = stringAddress;
					request->_ip = Wcs(addressMbs);
					request->_hostname = Wcs(hosttarget);
					request->_port = ntohs(port);
					request->_succeeded = true;
					
					service->OnAddressResolved();
				}
			}
		}
	}
}

/** DNSSDAttributeResolver **/
DNSSDAttributesRequest::DNSSDAttributesRequest(ref<DNSSDService> parent): DNSSDRequest(0,true), _parent(parent), _succeeded(false) {
}

DNSSDAttributesRequest::~DNSSDAttributesRequest() {
}

void DNSSDAttributesRequest::OnCreated() {
	ref<DNSSDService> service = _parent;
	if(service) {
		std::string fqdn = Mbs(service->GetQualifiedName());
		DNSServiceErrorType er = DNSServiceQueryRecord(&_service, kDNSServiceFlagsLongLivedQuery, service->_interface, fqdn.c_str(), kDNSServiceType_TXT, kDNSServiceClass_IN, (DNSServiceQueryRecordReply)Reply, reinterpret_cast<void*>(this));
		
		if(er!=kDNSServiceErr_NoError) {
			Log::Write(L"TJScout/DNSSDAttributesRequest", L"Could not resolve attributes for service; err="+Stringify(er)+L"; probably, Bonjour is not installed on this system");
		}
		else {
			DNSSDBrowserThread::Instance()->AddRequest(this);
		}
	}
}

void DNSSDAttributesRequest::Reply(DNSServiceRef DNSServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context) {
	DNSSDAttributesRequest* dar = reinterpret_cast<DNSSDAttributesRequest*>(context);
	if(dar!=0) {
		ref<DNSSDService> parent = dar->_parent;
		if(parent) {
			ThreadLock lock(&(parent->_lock));
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
					parent->_attributes[Wcs(key)] = Wcs(value);
				}
				index += length + 1;
			}
		
			dar->_succeeded = true;
			parent->OnAttributesResolved();
		}
	}
}

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
}

void DNSSDService::Resolve() {
	if(!_address) {
		std::string friendly = Mbs(_friendly);
		std::string type = Mbs(_type);
		std::string domain = Mbs(_domain);
		_address = GC::Hold(new DNSSDAddressRequest(this));
		_attributesRequest = GC::Hold(new DNSSDAttributesRequest(this));
		// DNSSDAddressRequest will add itself to the browser thread
	}
}

DNSSDService::~DNSSDService() {
}

String DNSSDService::CreateID(const String& name, const String& regtype, const String& domain) {
	return name + L"." + regtype + domain;
}

std::wstring DNSSDService::GetQualifiedName() const {
	return CreateID(_friendly, _type, _domain);
}

unsigned int DNSSDService::GetInterface() const {
	return _interface;
}

std::wstring DNSSDService::GetID() const {
	return L"dnssd:"+GetQualifiedName();
}

std::wstring DNSSDService::GetFriendlyName() const {
	return _friendly;
}

std::wstring DNSSDService::GetType() const {
	return _type;
}

std::wstring DNSSDService::GetAddress() const {
	if(_address && _address->_succeeded) {
		return _address->_ip;
	}
	Throw(L"Address for this service is not resolved yet!", ExceptionTypeError);
}

std::wstring DNSSDService::GetHostName() const {
	if(_address && _address->_succeeded) {
		return _address->_hostname;
	}
	Throw(L"Host name for this service is not resolved yet!", ExceptionTypeError);
}

unsigned short DNSSDService::GetPort() const {
	if(_address && _address->_succeeded) {
		return _address->_port;
	}
	Throw(L"Port for this service is not resolved yet!", ExceptionTypeError);
}

void DNSSDService::OnAddressResolved() {
	if(_address && _address->_succeeded && _attributesRequest && _attributesRequest->_succeeded) {
		EventResolved.Fire(this, DNSSDServiceResolvedNotification());
		
		// Start watching for TXT record changes
		_watcher = DNSSDUpdateRequest::Create(ref<DNSSDService>(this));
		DNSSDBrowserThread::Instance()->AddRequest(_watcher);
	}
}

void DNSSDService::OnAttributesResolved() {
	OnAddressResolved();
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
