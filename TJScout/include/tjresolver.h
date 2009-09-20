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
				virtual bool GetAttribute(const std::wstring& key, std::wstring& value);

			protected:
				virtual void SetAttribute(const std::wstring& key, const std::wstring& value);
				std::map< std::wstring, std::wstring > _attributes;
		};

		class SCOUT_EXPORTED RequestResolver: public virtual Object {
			public:
				virtual ~RequestResolver();
		};

		enum ServiceType {
			ServiceTypeDNSSD = 1,
		};

		class SCOUT_EXPORTED ServiceDescription {
			public:
				ServiceDescription();
				~ServiceDescription();
				void AddType(const ServiceType& type, const std::wstring& data);
				bool GetDescriptionOfType(const ServiceType& type, std::wstring& data) const;

			protected:
				std::map<ServiceType, std::wstring> _description;
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