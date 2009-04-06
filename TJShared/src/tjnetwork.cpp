#include "../include/tjcore.h"

#ifdef TJ_OS_WIN
	#include <iphlpapi.h>
#endif

#ifdef TJ_OS_MAC
	#include <unistd.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>

	typedef int SOCKET;
	#define INVALID_SOCKET -1
#endif

using namespace tj::shared;

std::string Networking::GetHostName() {
	ZoneEntry ze(Zones::NetworkZone);

	char buffer[256];
	
	#ifdef TJ_OS_WIN
		if(gethostname(buffer, 255)!=SOCKET_ERROR) {
			return std::string(buffer);
		}
	#endif
	
	#ifdef TJ_OS_MAC
		if(gethostname(buffer,255)==0) {
			return std::string(buffer);
		}
	#endif
	return "";
}

std::string Networking::GetHostAddress() {
	ZoneEntry ze(Zones::NetworkZone);

	std::string hostName = GetHostName();
	struct hostent *phe = gethostbyname(hostName.c_str());
    if(phe == 0) {
		return "";
    }

	if(phe->h_addr_list[0]==0) {
		return "";
	}

	in_addr address;
	memcpy(&address, phe->h_addr_list[0], sizeof(struct in_addr));
	return std::string(inet_ntoa(address));
}

void Networking::Wake(const MACAddress& mac) {
	ZoneEntry ze(Zones::NetworkZone);

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock!=INVALID_SOCKET) {
		sockaddr_in address;
		memset(&address, 0, sizeof(sockaddr_in));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(0xFFFFFFFF);
		address.sin_port = htons(9);

		unsigned char data[6*17];
		for(int a=0;a<17;a++) {
			for(int b=0;b<6;b++) {
				data[a*6+b] = mac.address.data[b];
			}
		}

		for(int b=0;b<6;b++) {
			data[b] = 0xFF;
		}

		int on = 1;
		if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(on))!=0) {
			Log::Write(L"TJShared/Networking", L"Could not call setsockopt for wake-on-lan");
		}
		if(sendto(sock, (const char*)data, 6*17, 0, (sockaddr*)&address, sizeof(address))<=0) {
			Log::Write(L"TJShared/Networking", L"Sendto returned zero in wake-on-lan attempt");
		}
		
		#ifdef TJ_OS_WIN
			closesocket(sock);
		#endif
		
		#ifdef TJ_OS_MAC
			shutdown(sock, SHUT_RDWR);
		#endif
	}
	else {
		Log::Write(L"TJShared/Networking", L"Could not open socket for wake-on-lan");
	}
}

bool Networking::GetMACAddress(const String& ip, MACAddress& maca) {
	ZoneEntry ze(Zones::NetworkZone);

	#ifdef WIN32
		IPAddr ipa = inet_addr(Mbs(ip).c_str());

		for(int a=0;a<6;a++) {
			maca.address.data[a] = 0xFF;
		}

		ULONG length = 6;
		if(SendARP(ipa, 0, (PULONG)maca.address.long_data, &length)==NO_ERROR) {
			return true;
		}
		return false;
	#else
		#warning Not implemented
		return false;
	#endif
}

Networking::MACAddress::MACAddress() {
	address.long_data[0] = 0L;
	address.long_data[1] = 0L;
}

Networking::MACAddress::MACAddress(const String& mac) {
	std::wistringstream is(mac);
	try {
		for(int a=0;a<6;a++) {
			is.width(2);
			int current = 0;
			is >> std::hex >> current;
			address.data[a] = (unsigned char)current;
			is.width(1);
			wchar_t semicolon = '\0';
			is >> semicolon;
		}
	}
	catch(...) {
		for(int a=0;a<6;a++) {
			address.data[a] = 0;
		}
	}
}

String Networking::MACAddress::ToString() const {
	std::wostringstream wos;
	for(int a=0;a<5;a++) {
		wos << std::setw(2) << std::setfill(L'0') << std::hex << std::uppercase << address.data[a] << L':';
	}
	wos << address.data[5];

	return wos.str();
}