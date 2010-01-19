#include "../include/tjservice.h"
#include "../include/tjdnssd.h"
using namespace tj::scout;

ref<ServiceRegistrationFactory> ServiceRegistrationFactory::_instance;

ServiceRegistrationFactory::ServiceRegistrationFactory() {
	RegisterPrototype(ServiceDiscoveryDNSSD, GC::Hold(new SubclassedPrototype<DNSSDServiceRegistration, ServiceRegistration>(L"DNS-SD")));
}

ServiceRegistrationFactory::~ServiceRegistrationFactory() {
}

strong<ServiceRegistrationFactory> ServiceRegistrationFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ServiceRegistrationFactory());
	}
	return _instance;
}

ref<ServiceRegistration> ServiceRegistrationFactory::CreateServiceRegistration(ServiceDiscoveryType discoveryType, const ServiceType& serviceType, const std::wstring& name, unsigned short port) {
	std::map<std::wstring,std::wstring> attrs;
	return CreateServiceRegistration(discoveryType, serviceType, name, port, attrs);
}

ref<ServiceRegistration> ServiceRegistrationFactory::CreateServiceRegistration(ServiceDiscoveryType discoveryType, const ServiceType& serviceType, const std::wstring& name, unsigned short port, const std::map<std::wstring,std::wstring>& attributes) {
	ref<ServiceRegistration> sr = CreateObjectOfType(discoveryType);
	if(sr) {
		sr->Register(serviceType, name, port, attributes);
	}
	
	return sr;
}

ServiceRegistration::ServiceRegistration(ServiceDiscoveryType dtp): _discoveryType(dtp) {
}

ServiceRegistration::~ServiceRegistration() {
}

ServiceDiscoveryType ServiceRegistration::GetDiscoveryType() const {
	return _discoveryType;
}

/** ServiceDescription **/
ServiceDescription::ServiceDescription() {
}

ServiceDescription::~ServiceDescription() {
}

void ServiceDescription::AddType(const ServiceDiscoveryType& type, const std::wstring& data) {
	_description[type] = data;
}

bool ServiceDescription::GetDescriptionOfType(const ServiceDiscoveryType& type, std::wstring& data) const {
	std::map<ServiceDiscoveryType,std::wstring>::const_iterator it = _description.find(type);
	if(it!=_description.end()) {
		data = it->second;
		return true;
	}
	return false;
}

/** Service **/
bool Service::GetAttribute(const std::wstring& key, std::wstring& value) {
	std::map<std::wstring, std::wstring>::const_iterator it = _attributes.find(key);
	if(it!=_attributes.end()) {
		value = it->second;
		return true;
	}
	return false;
}

void Service::SetAttribute(const std::wstring& key, const std::wstring& value) {
	_attributes[key] = value;
}
