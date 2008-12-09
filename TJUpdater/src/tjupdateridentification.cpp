#include "../include/tjupdater.h"
#include <iphlpapi.h>
using namespace tj::updater;

void Machine::GetUniqueIdentifier(std::wstring& mac) {
  IP_ADAPTER_INFO adapterInfo[16];
  DWORD size = sizeof(adapterInfo);

  if(ERROR_SUCCESS == GetAdaptersInfo(adapterInfo,&size)) {
	  std::wostringstream wos;

	  for(unsigned int a=0;a<adapterInfo[0].AddressLength;a++) {
		  wos << std::setw(2) << std::hex << std::setfill(L'0') << adapterInfo[0].Address[a];
		  if(a<(adapterInfo[0].AddressLength-1)) {
			wos << L'-';
		  }
	  }

	  mac = wos.str();
  }
}