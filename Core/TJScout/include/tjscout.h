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
 
 #ifndef _TJ_SCOUT_H
#define _TJ_SCOUT_H

#include "tjscoutinternal.h"
#include "tjresolver.h"

namespace tj {
	namespace scout {
		using namespace tj::shared;

		/** TJScout is the services broker for all applications and plug-ins. It provides a common
		interface for finding and resolving services on a network through protocols such as Bonjour
		(DNS-SD/mDNS) and SLP. Through TJScout, service resolvers can dynamically be added, and do
		not have to be recreated for each application or plug-in that wants to use the service location
		features.**/
		class SCOUT_EXPORTED Scout: public virtual Object {
			public:
				virtual ~Scout();
				virtual void Resolve(strong<ResolveRequest> rr);

				static strong<Scout> Instance();

			protected:
				Scout();
				std::deque< ref<Resolver> > _resolvers;
				static ref<Scout> _instance;
		};
	}
}

#endif