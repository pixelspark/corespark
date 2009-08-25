#ifndef _TJ_SCOUT_H
#define _TJ_SCOUT_H

#include "internal/tjscout.h"
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