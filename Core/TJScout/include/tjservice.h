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
 
 #ifndef _TJ_SCOUT_SERVICE_H
#define _TJ_SCOUT_SERVICE_H

#include "tjscoutinternal.h"

namespace tj {
	namespace scout {
		using namespace tj::shared;
		
		enum ServiceDiscoveryType {
			ServiceDiscoveryDNSSD = 1,
		};
		
		typedef std::wstring ServiceType;
		
		class SCOUT_EXPORTED Service: public virtual Object {
			public:
				struct UpdateNotification {
					std::wstring _attributeChanged;
				};
			
				Service();
				virtual ~Service();
				virtual std::wstring GetID() const = 0;
				virtual std::wstring GetFriendlyName() const = 0;
				virtual std::wstring GetType() const = 0;
				virtual std::wstring GetAddress() const = 0;
				virtual unsigned short GetPort() const = 0;
				virtual std::wstring GetHostName() const = 0;
				virtual bool GetAttribute(const std::wstring& key, std::wstring& value);
			
				Listenable<UpdateNotification> EventUpdate;
				
			protected:
				CriticalSection _lock;
				virtual void SetAttribute(const std::wstring& key, const std::wstring& value);
				std::map< std::wstring, std::wstring > _attributes;
		};
		
		class SCOUT_EXPORTED ServiceRegistration: public virtual Object {
			public:
				virtual ~ServiceRegistration();
				virtual ServiceDiscoveryType GetDiscoveryType() const;
				virtual void Register(const ServiceType& st, const std::wstring& serviceName, unsigned short port, const std::map<std::wstring, std::wstring>& attrs) = 0;
				virtual bool SetAttribute(const tj::shared::String& key, const tj::shared::String& value);
			
			protected:
				ServiceRegistration(ServiceDiscoveryType dtp);
				ServiceDiscoveryType _discoveryType;
		};
		
		class SCOUT_EXPORTED ServiceRegistrationFactory: public PrototypeBasedFactory<ServiceRegistration, ServiceDiscoveryType> {
			public:
				virtual ~ServiceRegistrationFactory();
				virtual ref<ServiceRegistration> CreateServiceRegistration(ServiceDiscoveryType discoveryType, const ServiceType& serviceType, const std::wstring& name, unsigned short port);
				virtual ref<ServiceRegistration> CreateServiceRegistration(ServiceDiscoveryType discoveryType, const ServiceType& serviceType, const std::wstring& name, unsigned short port, const std::map<std::wstring, std::wstring>& attrs);
			
				static strong<ServiceRegistrationFactory> Instance();
				
			protected:
				ServiceRegistrationFactory();
				static ref<ServiceRegistrationFactory> _instance;
		};
		
		class SCOUT_EXPORTED ServiceDescription {
			public:
				ServiceDescription();
				~ServiceDescription();
				void AddType(const ServiceDiscoveryType& type, const ServiceType& data);
				bool GetDescriptionOfType(const ServiceDiscoveryType& type, ServiceType& data) const;
				
			protected:
				std::map<ServiceDiscoveryType, ServiceType> _description;
		};
	}
}
	
#endif