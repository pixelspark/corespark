#include "../include/tjscout.h"
#include "../include/tjdnssd.h"

using namespace tj::shared;
using namespace tj::scout;

ref<Scout> Scout::_instance;

/** Scout **/
Scout::Scout() {
	_resolvers.push_back(GC::Hold(new DNSSDResolver()));
}

Scout::~Scout() {
}

strong<Scout> Scout::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new Scout());
	}
	return _instance;
}

void Scout::Resolve(strong<ResolveRequest> rr) {
	std::deque< ref<Resolver> >::iterator it = _resolvers.begin();
	while(it!=_resolvers.end()) {
		ref<Resolver> resolver = *it;
		if(resolver) {
			ref<RequestResolver> res = resolver->Resolve(rr);
			if(res) {
				rr->_requestResolvers.push_back(res);
			}
		}
		++it;
	}
}

/** Service **/
Service::Service() {
}

Service::~Service() {
}