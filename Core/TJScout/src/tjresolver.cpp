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
 
 #include "../include/tjresolver.h"
using namespace tj::shared;
using namespace tj::scout;

/** ResolveRequest **/
ResolveRequest::ResolveRequest(const ServiceDescription& type): _sd(type) {
}

ResolveRequest::~ResolveRequest() {
}

const ServiceDescription& ResolveRequest::GetDesiredServiceType() const {
	return _sd;
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