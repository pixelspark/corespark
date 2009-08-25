#ifndef _TJ_RESOLVER_H
#define _TJ_RESOLVER_H

#include "internal/tjscout.h"

namespace tj {
	namespace scout {
		using namespace tj::shared;

		class SCOUT_EXPORTED Service: public virtual Object {
			public:
				Service();
				virtual ~Service();
				virtual std::wstring GetID() const = 0;
				virtual std::wstring GetFriendlyName() const = 0;
				virtual std::wstring GetType() const = 0;
				virtual std::wstring GetAddress() const = 0;
				virtual unsigned short GetPort() const = 0;
		};

		class SCOUT_EXPORTED RequestResolver: public virtual Object {
			public:
				virtual ~RequestResolver();
		};

		class SCOUT_EXPORTED ResolveRequest: public virtual Object {
			friend class Scout;

			public:
				ResolveRequest(const std::wstring& desiredType);
				virtual ~ResolveRequest();
				virtual void Cancel();
				virtual void OnServiceFound(strong<Service> service);
				virtual void OnServiceDisappeared(const std::wstring& serviceID);
				virtual std::wstring GetDesiredServiceType() const;

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
				std::wstring _type;
		};

		class SCOUT_EXPORTED Resolver: public virtual Object {
			public:
				virtual ~Resolver();
				virtual ref<RequestResolver> Resolve(strong<ResolveRequest> rr) = 0;
		};
	}
}

#endif