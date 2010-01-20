#ifndef _TJ_DNSSD_RESOLVER_H
#define _TJ_DNSSD_RESOLVER_H

#include "tjscout.h"
#include "tjresolver.h"
#include <TJNP/include/tjsocket.h>

#ifdef TJ_OS_MAC
	#include <dns_sd.h>
#else
	#include <mDNS/include/dns_sd.h>
#endif

namespace tj {
	namespace scout {
		using namespace tj::shared;
		using namespace tj::np;
		
		class DNSSDBrowserThread;
		class DNSSDAddressFuture;
		class DNSSDUpdateRequest;
		
		class DNSSDRequest: public virtual Object, public SocketListener {
			friend class DNSSDBrowserThread;
			
			public:
				DNSSDRequest(DNSServiceRef service, bool oneShot);
				virtual ~DNSSDRequest();
				virtual void OnReceive(NativeSocket ns);
				DNSServiceRef _service;
				bool _oneShot;
				
			protected:
				weak<DNSSDBrowserThread> _thread;
		};
		
		class DNSSDBrowserThread: public SocketListenerThread {
			friend class DNSSDResolveRequest;
			
			public:
				DNSSDBrowserThread();
				virtual ~DNSSDBrowserThread();
				virtual void AddRequest(ref<DNSSDRequest> drq);
				virtual void RemoveRequest(ref<DNSSDRequest> drq);
				
				static strong<DNSSDBrowserThread> Instance();
				
			protected:
				CriticalSection _lock;
				std::set< ref<DNSSDRequest> > _requests;
				static ref<DNSSDBrowserThread> _instance;
		};

		class DNSSDService: public Service {
			public:
				DNSSDService(const std::wstring& friendly, const std::wstring& type, const std::wstring& domain, unsigned int iface);
				virtual ~DNSSDService();
				virtual std::wstring GetID() const;
				virtual std::wstring GetFriendlyName() const;
				virtual std::wstring GetType() const;
				virtual std::wstring GetAddress() const;
				virtual unsigned short GetPort() const;
				virtual std::wstring GetHostName() const;
				virtual std::wstring GetQualifiedName() const;
				virtual unsigned int GetInterface() const;
				virtual void SetAttribute(const String& key, const String& value);

			protected:
				virtual void OnCreated();
				std::wstring _type;
				std::wstring _friendly;
				std::wstring _domain;
				unsigned int _interface;
				ref< DNSSDUpdateRequest > _watcher;
				mutable ref<DNSSDAddressFuture> _address;
		};
		
		class DNSSDServiceRegistration: public ServiceRegistration {
			public:
				DNSSDServiceRegistration();
				virtual ~DNSSDServiceRegistration();
				virtual void Register(const ServiceType& st, const std::wstring& name, unsigned short port, const std::map<std::wstring, std::wstring>& attrs);
				virtual bool SetAttribute(const tj::shared::String& key, const tj::shared::String& value);
			
			private:
				std::map< tj::shared::String, tj::shared::String > _attributes;
				DNSServiceRef _sd;
		};
		
		class DNSSDUpdateRequest: public DNSSDRequest {			
			public:
				static strong<DNSSDUpdateRequest> Create(strong<DNSSDService> sd);
				virtual ~DNSSDUpdateRequest();
			static void Reply(DNSServiceRef DNSServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context);

			protected:
				DNSSDUpdateRequest(ref<DNSSDService> sd, DNSServiceRef service);
				virtual void OnCreated();
				weak<DNSSDService> _sd;
		};
		

		class DNSSDResolveRequest: public DNSSDRequest, public RequestResolver {
			friend class DNSSDBrowserThread;

			public:
				DNSSDResolveRequest(ref<ResolveRequest> rr, const std::wstring& type);
				virtual ~DNSSDResolveRequest();
				virtual ref<ResolveRequest> GetOriginalRequest();

			protected:
				virtual void OnCreated();
				weak<ResolveRequest> _request;
				std::wstring _type;
		};

		class DNSSDResolver: public Resolver {
			friend class DNSSDBrowserThread;

			public:
				DNSSDResolver();
				virtual ~DNSSDResolver();
				virtual ref<RequestResolver> Resolve(strong<ResolveRequest> rr);
		};
		
		class DNSSDAddressFuture: public Future {
			public:
				DNSSDAddressFuture(unsigned int iface, const char* name, const char* regtype, const char* domain);
				virtual ~DNSSDAddressFuture();	
				static void Reply(DNSServiceRef sdRef,DNSServiceFlags flags, uint32_t interfaceIndex,DNSServiceErrorType errorCode, const char* fullname,const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context);
				virtual void Run();
				virtual std::wstring GetAddress();
				virtual std::wstring GetHostName();
				virtual unsigned short GetPort();
				
			protected:
				struct ResolvedInfo {
					std::wstring _ip;
					std::wstring _hostname;
					unsigned short _port;
					bool _succeeded;
				};
				
				DNSServiceRef _service;
				ResolvedInfo _ri;
		};
	}
}

#endif