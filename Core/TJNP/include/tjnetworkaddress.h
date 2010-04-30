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
 
 #ifndef _TJ_NP_NETWORKADDRESS_H
#define _TJ_NP_NETWORKADDRESS_H

#include "tjnpinternal.h"

#ifdef TJ_OS_POSIX
	#include <arpa/inet.h>
#endif

#ifdef TJ_OS_WIN
	#include <Winsock2.h>
#endif

namespace tj {
	namespace np {
		enum AddressFamily {
			AddressFamilyNone = 0,
			AddressFamilyIPv4,
			AddressFamilyIPv6,
			_AddressFamilyLast,
		};

		enum TransportProtocol {
			TransportProtocolNone = 0,
			TransportProtocolTCP,
			TransportProtocolUDP,
			_TransportProtocolLast,
		};
		
		class NP_EXPORTED NetworkAddress {
			public:
				NetworkAddress(const tj::shared::String& spec, bool passive = false);
				~NetworkAddress();
				std::wstring ToString() const;
				AddressFamily GetAddressFamily() const;
				bool GetIPv6SocketAddress(sockaddr_in6* addr) const;
				bool GetIPv4SocketAddress(sockaddr_in* addr) const;
				
				
			protected:
				AddressFamily _family;
				sockaddr_in6 _address;
				sockaddr_in _v4address;
		};
	}
}

#endif