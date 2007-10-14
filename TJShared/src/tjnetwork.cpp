#include "../include/tjshared.h"
using namespace tj::shared;

std::string Networking::GetHostName() {
	char buffer[256];
	if(gethostname(buffer, 255)!=SOCKET_ERROR) {
		return std::string(buffer);
	}
	return "";
}

std::string Networking::GetHostAddress() {
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