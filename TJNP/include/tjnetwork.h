#ifndef _TJNETWORK_H
#define _TJNETWORK_H

#include "tjnpinternal.h"

#ifdef TJ_OS_MAC
	typedef struct in_addr;
#endif

#ifdef TJ_OS_LINUX
	#include <netinet/in.h>
#endif

namespace tj {
	namespace np {
		class NP_EXPORTED Networking {
			public:
				struct NP_EXPORTED MACAddress {
					MACAddress();
					MACAddress(const tj::shared::String& mac);

					union {
						unsigned char data[6];
						unsigned long long_data[2];
					} address;

					tj::shared::String ToString() const;
				};

				static std::string GetHostName();
				static std::string GetHostAddress();
				static void Wake(const MACAddress& mac);
				static bool GetMACAddress(const tj::shared::String& ip, MACAddress& maca);
				static tj::shared::String GetHostName(const tj::shared::String& ip);
				static tj::shared::String GetHostName(const in_addr* address);
		};
	}
}

#endif
