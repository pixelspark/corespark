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