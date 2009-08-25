#include "../include/tjresolver.h"
using namespace tj::shared;
using namespace tj::scout;

/** ResolveRequest **/
ResolveRequest::ResolveRequest(const std::wstring& type): _type(type) {
}

ResolveRequest::~ResolveRequest() {
}

std::wstring ResolveRequest::GetDesiredServiceType() const {
	return _type;
}

void ResolveRequest::Cancel() {
	EventCancelled.Fire(this, CancelNotification());
}

void ResolveRequest::OnServiceFound(strong<Service> service) {
	{
		ThreadLock lock(&_lock);
		_services[service->GetID()] = service;
	}
	EventService.Fire(this, ServiceNotification(true, service));
}

void ResolveRequest::OnServiceDisappeared(const std::wstring& id) {
	ref<Service> service;

	{
		ThreadLock lock(&_lock);
		std::map<std::wstring, ref<Service> >::iterator it = _services.find(id);
		if(it!=_services.end()) {
			service = it->second;
			_services.erase(it);
		}
	}

	if(service) {
		EventService.Fire(this, ServiceNotification(false, service));
	}
}

ResolveRequest::ServiceNotification::ServiceNotification(bool o, strong<Service> ss): online(o), service(ss) {
}

/** Resolver **/
Resolver::~Resolver() {
}

/** RequestResolver **/
RequestResolver::~RequestResolver() {
}