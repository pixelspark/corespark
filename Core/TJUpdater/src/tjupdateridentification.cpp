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