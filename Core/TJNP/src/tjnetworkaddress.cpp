#include "../include/tjnetworkaddress.h"
#include "../include/tjsocket.h"
#include <errno.h>

#ifdef TJ_OS_POSIX
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
#endif

#ifdef TJ_OS_WIN
	#include <Winsock2.h>
#endif

using namespace tj::shared;
using namespace tj::np;

/** NetworkAddress **/
NetworkAddress::NetworkAddress(const String& s, bool passive): _family(AddressFamilyNone) {
	NetworkInitializer ni;

	if(s.length()>0) {
		memset(&_address, 0, sizeof(_address));
		addrinfo* firstResult;
		addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = PF_UNSPEC;
		hints.ai_flags = AI_V4MAPPED|(passive ? AI_PASSIVE : 0)|AI_ADDRCONFIG;
		hints.ai_socktype = 0;
		hints.ai_protocol = 0;
		
		bool success = false;
		std::string mbs = Mbs(s);
		int r = getaddrinfo(mbs.c_str(), NULL, &hints, &firstResult);
		if(r==0) {
			if(firstResult!=NULL) {
				if(firstResult->ai_family==AF_INET6) {
					_family = AddressFamilyIPv6;
					memcpy(&_address, firstResult->ai_addr, sizeof(sockaddr_in6));
					success = true;
				}
				else if(firstResult->ai_family==AF_INET) {
					_family = AddressFamilyIPv4;
					memcpy(&_v4address, firstResult->ai_addr, sizeof(sockaddr_in));
					success = true;
				}
				freeaddrinfo(firstResult);
			}
		}
		else {
			#ifdef TJ_OS_POSIX
			std::wstring error = Wcs(std::string(gai_strerror(r)));
			#endif
			
			#ifdef TJ_OS_WIN
			std::wstring error = std::wstring(gai_strerror(r));
			#endif
			
			Log::Write(L"TJNP/NetworkAddress", L"getaddrinfo() failed: " + error);
		}
	}
	else {
		if(passive) {
			_family = AddressFamilyIPv6;
			in6_addr any = IN6ADDR_ANY_INIT;
			_address.sin6_addr = any;
			_v4address.sin_addr.s_addr = INADDR_ANY;
		}
	}
}

NetworkAddress::~NetworkAddress() {
}

AddressFamily NetworkAddress::GetAddressFamily() const {
	return _family;
}

bool NetworkAddress::GetIPv6SocketAddress(sockaddr_in6* addr) const {
	if(_family==AddressFamilyIPv6) {
		memcpy(addr, &_address, sizeof(_address));
		return true;
	}
	return false;
}

bool NetworkAddress::GetIPv4SocketAddress(sockaddr_in* addr) const {
	if(_family==AddressFamilyIPv4) {
		memcpy(addr, &_v4address, sizeof(_v4address));
		return true;
	}
	return false;
}

std::wstring NetworkAddress::ToString() const {
	if(_family==AddressFamilyNone) {
		return L"";
	}
	else if(_family==AddressFamilyIPv6) {
		#ifdef TJ_OS_POSIX
			char buffer[255];
			memset(buffer, 0, sizeof(char)*255);
			std::string friendlyAddress = inet_ntop(AF_INET6, (void*)&(_address.sin6_addr), buffer, 255);
			return Wcs(friendlyAddress)+L'%'+StringifyHex(_address.sin6_scope_id);
		#endif

		#ifdef TJ_OS_WIN
			wchar_t buffer[255];
			memset(buffer, 0, sizeof(wchar_t)*255);
			DWORD bufferSize = 255;
			int r = WSAAddressToString(const_cast<LPSOCKADDR>((const SOCKADDR*)&(_address)), sizeof(sockaddr_in6), NULL, buffer, &bufferSize);
			if(r==0) {
				return std::wstring(buffer);
			}
			else {
				//Log::Write(L"TJNP/NetworkAddress", L"Failed to stringify IPv6 address; err="+Stringify(GetLastError()));
			}
		#endif
	}
	else if(_family==AddressFamilyIPv4) {
		#ifdef TJ_OS_POSIX
			char buffer[255];
			memset(buffer, 0, sizeof(char)*255);
			std::string friendlyAddress = inet_ntop(AF_INET, (void*)&(_v4address.sin_addr), buffer, 255);
			return Wcs(friendlyAddress);
		#endif

		#ifdef TJ_OS_WIN
			wchar_t buffer[255];
			memset(buffer, 0, sizeof(wchar_t)*255);
			DWORD bufferSize = 255;
			int r = WSAAddressToString(const_cast<LPSOCKADDR>((const SOCKADDR*)&_v4address), sizeof(sockaddr_in), NULL, buffer, &bufferSize);
			if(r==0) {
				return std::wstring(buffer);
			}
			else {
				//Log::Write(L"TJNP/NetworkAddress", L"Failed to stringify IPv4 address; err="+Stringify(GetLastError()));
			}
		#endif

	}
	return L"[???]";
}