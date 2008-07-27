#ifndef _TJZONE_H
#define _TJZONE_H

namespace tj {
	namespace shared {
		class EXPORTED Zone {
			friend class ZoneDenial;
			friend class ZoneEntry;

			public:
				Zone();
				~Zone();
				bool CanEnter();

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


		class EXPORTED ZoneEntry {
			public:
				inline ZoneEntry(Zone& z): _zone(z) {
					_zone.Enter();
				}

				inline ~ZoneEntry() {
					_zone.Leave();
				}

			private:
				Zone& _zone;
		};

		/** Some global zones that can be used everywhere */
		class EXPORTED Zones {
			public:
				static Zone LocalFileReadZone;
				static Zone LocalFileWriteZone;
				static Zone LocalFileInfoZone;				// Retrieval of information such as size, date etc. about local files
				static Zone LocalFileAdministrationZone;	// Deleting, renaming etc. local files
				static Zone LogZone;						// Writing to the log (e.g. Log::Write).
				static Zone ModifyLocaleZone;				// Loading/changing loaded locale strings
				static Zone ClipboardZone;					// Get/set clipboard contents
				static Zone ShowModalDialogsZone;			// Show modal messages/dialogs
		};
	}
}

#endif