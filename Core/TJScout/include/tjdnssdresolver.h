/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_DNSSD_RESOLVER_H
#define _TJ_DNSSD_RESOLVER_H

#include "tjscout.h"
#include "tjresolver.h"
#include <mDNS/include/dns_sd.h>

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
				unsigned int _interface;
				std::wstring _address;
				unsigned short _port;
		};

		class DNSSDResolveRequest: public RequestResolver {
			friend class DNSSDBrowserThread;

			public:
				DNSSDResolveRequest(ref<ResolveRequest> rr, const std::wstring& type);
				virtual ~DNSSDResolveRequest();
				virtual ref<ResolveRequest> GetOriginalRequest();

			protected:
				ref<DNSSDBrowserThread> _thread;
				DNSServiceRef _service;
				weak<ResolveRequest> _request;
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