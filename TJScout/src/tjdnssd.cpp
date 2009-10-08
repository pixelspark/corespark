#include "../include/tjdnssd.h"

#ifdef TJ_OS_WIN
	#include "../../Libraries/mDNS/include/dns_sd.h"
#endif

#ifdef TJ_OS_MAC
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
			
				#ifdef TJ_OS_POSIX
					int _controlSocket[2];
				#endif
			
				#ifdef TJ_OS_WIN
					Event _cancelledEvent;
				#endif
		};
	}
}

DNSSDBrowserThread::DNSSDBrowserThread(DNSServiceRef service): _service(service) {
	#ifdef TJ_OS_POSIX
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, _controlSocket)!=0) {
			Log::Write(L"TJScout/DNSSDBrowserThread", L"Could not create control socket pair");
		}
	#endif
}

DNSSDBrowserThread::~DNSSDBrowserThread() {
	Cancel();
	WaitForCompletion();
	DNSServiceRefDeallocate(_service);
	
	#ifdef TJ_OS_POSIX
		close(_controlSocket[0]);
		close(_controlSocket[1]);
	#endif
}

void DNSSDBrowserThread::Run() {
	int fd = DNSServiceRefSockFD(_service);

	#ifdef TJ_OS_WIN
		while(!_cancelledEvent.Wait(Time(1000))) {
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
	#endif
	
	
	#ifdef TJ_OS_POSIX
		while(true) {
			fd_set fdset;
			FD_ZERO(&fdset);
			FD_SET(fd, &fdset);
			FD_SET(_controlSocket[1], &fdset);
			select(Util::Max(_controlSocket[1],fd)+1, &fdset, 0, 0, NULL);
			if(FD_ISSET(_controlSocket[1], &fdset)) {
				return;
			}
			else if(FD_ISSET(fd, &fdset)) {
				DNSServiceProcessResult(_service);
			}
			else {
				// Something weird?
				return;
			}
		}
	#endif
}

void DNSSDBrowserThread::Cancel() {
	#ifdef TJ_OS_WIN
		_cancelledEvent.Signal();
	#endif
	
	#ifdef TJ_OS_POSIX
		char quit[1] = {'Q'};
		if(write(_controlSocket[0], quit, 1)==-1) {
			Log::Write(L"TJNP/SocketListenerThread", L"Could not send quit message to listener thread");
		}
	#endif
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
	if(sd.GetDescriptionOfType(ServiceDiscoveryDNSSD,data)) {
		return GC::Hold(new DNSSDResolveRequest(rr,data));
	}
	return null;
}

/** DNSSDAddressResolver **/
class DNSSDAddressResolver {
	public:
		struct ResolvedInfo {
			std::wstring _ip;
			std::wstring _hostname;
			unsigned short _port;
			bool _succeeded;
		};

		static void Reply(DNSServiceRef sdRef,DNSServiceFlags flags, uint32_t interfaceIndex,DNSServiceErrorType errorCode, const char* fullname,const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context) {
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

		static bool ResolveAddressForService(std::wstring& address, unsigned short& port, unsigned int iface, const char* name, const char* regtype, const char* domain, std::wstring& hostname) {
			DNSServiceRef service = 0;
			ResolvedInfo ri;
			ri._succeeded = false;
			DNSServiceResolve(&service, 0, iface, name, regtype, domain, (DNSServiceResolveReply)Reply, reinterpret_cast<void*>(&ri));
			DNSServiceProcessResult(service);
			DNSServiceRefDeallocate(service);

			if(ri._succeeded) {
				address = ri._ip;
				port = ri._port;
				hostname = ri._hostname;
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
	if(!DNSSDAddressResolver::ResolveAddressForService(_address, _port, iface, Mbs(friendly).c_str(), Mbs(type).c_str(), Mbs(domain).c_str(), _hostname)) {
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

std::wstring DNSSDService::GetHostName() const {
	return _hostname;
}

unsigned short DNSSDService::GetPort() const {
	return _port;
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

void DNSSDServiceRegistration::Register(const ServiceType& st, const std::wstring& name, unsigned short port, const std::map<std::wstring, std::wstring>& attributes) {
	TXTRecordRef tr;
	unsigned char buffer[1024];
	TXTRecordCreate(&tr, 1024, (void*)buffer);
	
	std::map<std::wstring, std::wstring>::const_iterator it = attributes.begin();
	while(it!=attributes.end()) {
		std::string value = Mbs(it->second);
		TXTRecordSetValue(&tr, Mbs(it->first).c_str(), value.length(), value.c_str());
		++it;
	}
	
	DNSServiceRegister(&_sd, 0, 0, Mbs(name).c_str(), Mbs(st).c_str(), NULL, NULL, htons(port), TXTRecordGetLength(&tr), TXTRecordGetBytesPtr(&tr), NULL, NULL);
	TXTRecordDeallocate(&tr);
}