#ifndef _TJ_DNSSD_RESOLVER_H
#define _TJ_DNSSD_RESOLVER_H

#include "tjscout.h"
#include "tjresolver.h"

#ifdef TJ_OS_MAC
	#include <dns_sd.h>
#else
	#include <mDNS/include/dns_sd.h>
#endif

namespace tj {
	namespace scout {
		using namespace tj::shared;
		class DNSSDBrowserThread;

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

			protected:
				std::wstring _type;
				std::wstring _friendly;
				std::wstring _domain;
				std::wstring _hostname;
				unsigned int _interface;
				std::wstring _address;
				unsigned short _port;
		};
		
		class DNSSDServiceRegistration: public ServiceRegistration {
			public:
				DNSSDServiceRegistration();
				virtual ~DNSSDServiceRegistration();
				virtual void Register(const ServiceType& st, const std::wstring& name, unsigned short port, const std::map<std::wstring, std::wstring>& attrs);
			
			private:
				DNSServiceRef _sd;
		};

		class DNSSDResolveRequest: public RequestResolver {
			friend class DNSSDBrowserThread;

			public:
				DNSSDResolveRequest(ref<ResolveRequest> rr, const std::wstring& type);
				virtual ~DNSSDResolveRequest();
				virtual ref<ResolveRequest> GetOriginalRequest();

			protected:
				virtual void OnCreated();
			
				ref<DNSSDBrowserThread> _thread;
				DNSServiceRef _service;
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
	}
}

#endif