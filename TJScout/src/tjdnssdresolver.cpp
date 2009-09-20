#include "../include/tjdnssdresolver.h"
#include "../../Libraries/mDNS/include/dns_sd.h"
using namespace tj::shared;
using namespace tj::scout;

/** DNSSDBrowserThread **/
namespace tj {
	namespace scout {
		class DNSSDBrowserThread: public Thread {
			friend class DNSSDResolveRequest;
			public:
				DNSSDBrowserThread(DNSServiceRef service);
				virtual ~DNSSDBrowserThread();
				virtual void Run();
				virtual void Cancel();

			protected:
				DNSServiceRef _service;
				Event _cancelledEvent;
		};
	}
}

DNSSDBrowserThread::DNSSDBrowserThread(DNSServiceRef service): _service(service) {
}

DNSSDBrowserThread::~DNSSDBrowserThread() {
	Cancel();
	WaitForCompletion();
	DNSServiceRefDeallocate(_service);
}

void DNSSDBrowserThread::Run() {
	int fd = DNSServiceRefSockFD(_service);

	while(WaitForSingleObject(_cancelledEvent.GetHandle(), 1000)==WAIT_TIMEOUT) {
		TIMEVAL tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1;

		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(fd, &fdset);

		int sr = select(0, &fdset, 0, 0, &tv);
		if(sr==SOCKET_ERROR) {
			Log::Write(L"TJScout/DNSSDBrowserThread", L"SOCKET_ERROR on select!");
			return;
		}
		else if(sr>0) {
			DNSServiceProcessResult(_service);
		}
		else {
			// Nothing to be processed, wait another second
		}
	}
}

void DNSSDBrowserThread::Cancel() {
	_cancelledEvent.Signal();
}

/** DNSSDResolveRequest **/
void DNSSDDiscoveryBrowseReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regType, const char *replyDomain, void *context)  {
	if(context!=0) {
		ref<DNSSDResolveRequest> rr = ref<DNSSDResolveRequest>((DNSSDResolveRequest*)context);
		if(rr) {
			ref<ResolveRequest> req = rr->GetOriginalRequest();
			if(req) {
				ref<Service> service = GC::Hold(new DNSSDService(Wcs(serviceName), Wcs(regType), Wcs(replyDomain), interfaceIndex));

				Log::Write(L"TJScout/DNSSDDiscovery", L"Service '"+Wcs(serviceName)+L"' (type="+Wcs(regType)+L" flags="+StringifyHex(flags)+L" iface="+Stringify(interfaceIndex)+L")");

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

DNSSDResolveRequest::DNSSDResolveRequest(ref<ResolveRequest> rq, const std::wstring& type): _service(0), _request(rq) {
	if(rq) {
		std::string serviceType = Mbs(type);
		DNSServiceErrorType er = DNSServiceBrowse(&_service, 0, 0, serviceType.c_str(), NULL, (DNSServiceBrowseReply)DNSSDDiscoveryBrowseReply, (void*)this);
		if(er!=kDNSServiceErr_NoError) {
			Log::Write(L"TJScout/DNSSDResolveRequest", L"Could not start browsing for services; err="+Stringify(er)+L"; probably, Bonjour is not installed on this system");
		}
		else {
			_thread = GC::Hold(new DNSSDBrowserThread(_service));
			_thread->Start();
		}
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
	if(sd.GetDescriptionOfType(ServiceTypeDNSSD,data)) {
		return GC::Hold(new DNSSDResolveRequest(rr,data));
	}
}

/** DNSSDAddressResolver **/
class DNSSDAddressResolver {
	public:
		struct ResolvedInfo {
			std::wstring _ip;
			unsigned short _port;
			bool _succeeded;
		};

		static void Reply(DNSServiceRef sdRef,DNSServiceFlags flags, uint32_t interfaceIndex,DNSServiceErrorType errorCode, const char* fullname,const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context) {
			ResolvedInfo* ri = (ResolvedInfo*)context;
			if(ri!=0) {
				ri->_succeeded = false;
				hostent* he = gethostbyname(hosttarget);
				if(he!=0) {
					struct in_addr addr;
					addr.s_addr = *(u_long *)he->h_addr_list[0];
					char* stringAddress = inet_ntoa(addr);
					ri->_ip = Wcs(std::string(stringAddress));
					ri->_port = ntohs(port);
					ri->_succeeded = true;
				}
			}
		}

		static bool ResolveAddressForService(std::wstring& address, unsigned short& port, unsigned int iface, const char* name, const char* regtype, const char* domain) {
			DNSServiceRef service = 0;
			ResolvedInfo ri;
			ri._succeeded = false;
			DNSServiceResolve(&service, 0, iface, name, regtype, domain, (DNSServiceResolveReply)Reply, &ri);
			DNSServiceProcessResult(service);
			DNSServiceRefDeallocate(service);

			if(ri._succeeded) {
				address = ri._ip;
				port = ri._port;
				return true;
			}
			return false;
		}
};

/** DNSSDAddressResolver **/
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
			DNSServiceQueryRecord(&service, 0, iface, fqdn.c_str(), kDNSServiceType_TXT, kDNSServiceClass_IN, (DNSServiceQueryRecordReply)Reply, &ri);
			DNSServiceProcessResult(service);
			DNSServiceRefDeallocate(service);
			return ri.succeeded;
		}
};

/** DNSSDService **/
DNSSDService::DNSSDService(const std::wstring& friendly, const std::wstring& type, const std::wstring& domain, unsigned int iface): _friendly(friendly), _type(type), _domain(domain), _interface(iface), _port(0) {
	if(!DNSSDAddressResolver::ResolveAddressForService(_address, _port, iface, Mbs(friendly).c_str(), Mbs(type).c_str(), Mbs(domain).c_str())) {
		Log::Write(L"TJScout/DNSSDService", L"Could not resolve address for service "+GetID());
	}

	/* Query TXT record containing attributes (a TXT record MUST always be present, hence we can use a blocking query here,
	this also means that non-conforming mDNS responders can block us...) */
	std::wstring name = friendly + L"." + _type + _domain;
	if(!DNSSDAttributeResolver::ResolveAttributesForService(iface, name, _attributes)) {
		Log::Write(L"TJScout/DNSSDService", L"Could not resolve attributes for service"+GetID());
	}
}

DNSSDService::~DNSSDService() {
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
	return _address;
}

unsigned short DNSSDService::GetPort() const {
	return _port;
}