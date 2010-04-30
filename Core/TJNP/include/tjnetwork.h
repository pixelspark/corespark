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
