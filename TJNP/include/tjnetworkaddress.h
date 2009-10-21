#ifndef _TJ_NP_NETWORKADDRESS_H
#define _TJ_NP_NETWORKADDRESS_H

#include "internal/tjnp.h"

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