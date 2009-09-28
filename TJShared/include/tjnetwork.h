#ifndef _TJNETWORK_H
#define _TJNETWORK_H

#ifdef TJ_OS_MAC
	typedef struct in_addr;
#endif

#ifdef TJ_OS_LINUX
	#include <netinet/in.h>
#endif

namespace tj {
	namespace shared {
		class EXPORTED Networking {
			public:
				struct EXPORTED MACAddress {
					MACAddress();
					MACAddress(const String& mac);

					union {
						unsigned char data[6];
						unsigned long long_data[2];
					} address;

					String ToString() const;
				};

				static std::string GetHostName();
				static std::string GetHostAddress();
				static void Wake(const MACAddress& mac);
				static bool GetMACAddress(const String& ip, MACAddress& maca);
				static String GetHostName(const String& ip);
				static String GetHostName(const in_addr* address);
		};
	}
}

#endif
