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
 
 #ifndef _TJ_RESOLVER_H
#define _TJ_RESOLVER_H

#include "tjscoutinternal.h"
#include "tjservice.h"

namespace tj {
	namespace scout {
		using namespace tj::shared;

		class SCOUT_EXPORTED RequestResolver: public virtual Object {
			public:
				virtual ~RequestResolver();
		};

		class SCOUT_EXPORTED ResolveRequest: public virtual Object {
			friend class Scout;

			public:
				ResolveRequest(const ServiceDescription& desc);
				virtual ~ResolveRequest();
				virtual void Cancel();
				virtual void OnServiceFound(strong<Service> service);
				virtual void OnServiceDisappeared(const std::wstring& serviceID);
				virtual const ServiceDescription& GetDesiredServiceType() const;

				struct CancelNotification {
				};

				Listenable<CancelNotification> EventCancelled;

				struct ServiceNotification {
					ServiceNotification(bool online, strong<Service> service);
					bool online;
					strong<Service> service;
				};

				Listenable<ServiceNotification> EventService;

			protected:
				CriticalSection _lock;
				std::map< std::wstring, ref<Service> > _services;
				std::deque< ref<RequestResolver> > _requestResolvers;
				ServiceDescription _sd;
		};

		class SCOUT_EXPORTED Resolver: public virtual Object {
			public:
				virtual ~Resolver();
				virtual ref<RequestResolver> Resolve(strong<ResolveRequest> rr) = 0;
		};
	}
}

#endif