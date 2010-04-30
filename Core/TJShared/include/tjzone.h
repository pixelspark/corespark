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
 
 #ifndef _TJZONE_H
#define _TJZONE_H

#include "tjsharedinternal.h"
#include "tjthread.h"

namespace tj {
	namespace shared {
		class EXPORTED Zone {
			friend class ZoneDenial;
			friend class ZoneEntry;

			public:
				Zone();
				~Zone();
				bool CanEnter();
				bool IsInside() const;

				// For 'sandboxed' threads
				static void GlobalDeny();
				static void GlobalAllow();

			protected:
				void Enter();
				void Leave();
				void Deny();
				void Allow();

			private:
				ThreadLocal _entry;
				ThreadLocal _barrier;
				static ThreadLocal _globalBarrier;
		};

		class EXPORTED ZoneDenial {
			public:
				inline ZoneDenial(Zone& z): _zone(z) {
					_zone.Deny();
				}

				inline ~ZoneDenial() {
					_zone.Allow();
				}

			private:
				Zone& _zone;
		};

		/** Some global zones that can be used everywhere */
		class EXPORTED Zones {
			public:
				enum PredefinedZone {
					LocalFileReadZone = 1,
					LocalFileWriteZone,
					LocalFileInfoZone,
					LocalFileAdministrationZone,
					LogZone,
					ModifyLocaleZone,
					ClipboardZone,
					ShowModalDialogsZone,
					NetworkZone,
					NetworkServerZone,
					DebugZone,
					_LastZone,
				};

				static Zone& Get(const PredefinedZone& z);
				static bool IsDebug(); // Returns true when inside the 'debug zone', which means we should for example print more debug stuff
		};

		class EXPORTED ZoneEntry {
			public:
				inline ZoneEntry(Zone& z): _zone(z) {
					_zone.Enter();
				}

				inline ZoneEntry(const Zones::PredefinedZone& pz): _zone(Zones::Get(pz)) {
					_zone.Enter();
				}

				inline ~ZoneEntry() {
					_zone.Leave();
				}

			private:
				Zone& _zone;
		};
	}
}

#endif