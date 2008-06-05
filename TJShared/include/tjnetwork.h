#ifndef _TJNETWORK_H
#define _TJNETWORK_H

namespace tj {
	namespace shared {
		class EXPORTED Networking {
			public:
				struct EXPORTED MACAddress {
					MACAddress();
					MACAddress(const std::wstring& mac);

					union {
						unsigned char data[6];
						unsigned long long_data[2];
					} address;

					std::wstring ToString() const;
				};

				static std::string GetHostName();
				static std::string GetHostAddress();
				static void Wake(const MACAddress& mac);
				static bool GetMACAddress(const std::wstring& ip, MACAddress& maca);
		};
	}
}

#endif