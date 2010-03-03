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
		class DNSSDAddressRequest;
		class DNSSDUpdateRequest;
		class DNSSDAttributesRequest;
		
		class DNSSDRequest: public virtual Object, public SocketListener {
			friend class DNSSDBrowserThread;
			
			public:
				DNSSDRequest(DNSServiceRef service, bool oneShot);
				virtual ~DNSSDRequest();
				virtual void OnReceive(NativeSocket ns);
				DNSServiceRef _service;
				bool _oneShot;
				
			protected:
				bool _dirty;
				weak<DNSSDBrowserThread> _thread;
		};
		
		class DNSSDBrowserThread: public SocketListenerThread {
			friend class DNSSDResolveRequest;
			
			public:
				DNSSDBrowserThread();
				virtual ~DNSSDBrowserThread();
				virtual void AddRequest(ref<DNSSDRequest> drq);
				virtual void RemoveRequest(NativeSocket ns);
				
				static strong<DNSSDBrowserThread> Instance();
				
			protected:
				static ref<DNSSDBrowserThread> _instance;
		};

		class DNSSDService: public Service {
			friend class DNSSDAddressRequest;
			friend class DNSSDAttributesRequest;
			
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
			
				virtual void Resolve();
				struct DNSSDServiceResolvedNotification {};
				Listenable<DNSSDServiceResolvedNotification> EventResolved;
			
				static String CreateID(const std::wstring& name, const std::wstring& regtype, const std::wstring& domain);

			protected:
				virtual void OnAddressResolved();
				virtual void OnAttributesResolved();
				virtual void OnCreated();
			
				std::wstring _type;
				std::wstring _friendly;
				std::wstring _domain;
				unsigned int _interface;
				ref<DNSSDUpdateRequest> _watcher;
				ref<DNSSDAddressRequest> _address;
				ref<DNSSDAttributesRequest> _attributesRequest;
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
		

		class DNSSDResolveRequest: public DNSSDRequest, public RequestResolver, public tj::shared::Listener<DNSSDService::DNSSDServiceResolvedNotification> {
			friend class DNSSDBrowserThread;

			public:
				DNSSDResolveRequest(ref<ResolveRequest> rr, const std::wstring& type);
				virtual ~DNSSDResolveRequest();
				virtual void AddNewService(strong<DNSSDService> ds);
				virtual void OnServiceDisappeared(const tj::shared::String& ident);
				virtual void Notify(ref<Object> source, const DNSSDService::DNSSDServiceResolvedNotification& data);

			protected:
				virtual void OnCreated();
			
				CriticalSection _lock;
				weak<ResolveRequest> _request;
				std::wstring _type;
				std::set< ref<DNSSDService> > _waitingToResolve;
		};

		class DNSSDResolver: public Resolver {
			friend class DNSSDBrowserThread;

			public:
				DNSSDResolver();
				virtual ~DNSSDResolver();
				virtual ref<RequestResolver> Resolve(strong<ResolveRequest> rr);
		};
		
		class DNSSDAttributesRequest: public DNSSDRequest {
			friend class DNSSDService;
			
		public:
			DNSSDAttributesRequest(ref<DNSSDService> parent);
			virtual ~DNSSDAttributesRequest();	
			virtual void OnCreated();
			static void Reply(DNSServiceRef DNSServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context);
				
		protected:
			weak<DNSSDService> _parent;
			bool _succeeded;
		};
		
		class DNSSDAddressRequest: public DNSSDRequest {
			friend class DNSSDService;
			
			public:
				DNSSDAddressRequest(ref<DNSSDService> parent);
				virtual ~DNSSDAddressRequest();	
				virtual void OnCreated();
				static void Reply(DNSServiceRef sdRef,DNSServiceFlags flags, uint32_t interfaceIndex,DNSServiceErrorType errorCode, const char* fullname,const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context);
				
			protected:
				weak<DNSSDService> _parent;
				
				std::wstring _ip;
				std::wstring _hostname;
				unsigned short _port;
				bool _succeeded;
		};
	}
}

#endif