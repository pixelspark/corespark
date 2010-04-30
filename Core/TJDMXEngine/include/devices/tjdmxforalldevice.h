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
 
 #ifdef TJ_OS_WIN
	#ifndef _TJDMXFORALLDEVICE_H
	#define _TJDMXFORALLDEVICE_H

	#include "../tjdmxinternal.h"
	#include "../tjdmxdevice.h"

	namespace tj {
		namespace dmx {
			namespace devices {
				using namespace tj::shared;
				using namespace tj::dmx;

				class DMX_EXPORTED DMXForAllDevice: public DMXDevice {
					friend class DMXController;

					public:
						DMXForAllDevice(HMODULE module);
						virtual ~DMXForAllDevice();
						virtual std::wstring GetPort();
						virtual std::wstring GetDeviceID();
						virtual std::wstring GetDeviceInfo();
						virtual std::wstring GetDeviceSerial();
						virtual std::wstring GetDeviceName() const;
						virtual unsigned int GetSupportedUniversesCount();

						#ifdef TJ_DMX_HAS_TJSHAREDUI
							virtual ref<PropertySet> GetProperties();
						#endif
						
						virtual void Save(TiXmlElement* you);
						virtual void Load(TiXmlElement* you);
						
					protected:
						virtual void Connect();
						virtual void OnTransmit(ref<DMXController> controller);

						// Procedures in the DLL
						struct VERSION_INFO {
							char		InterfaceInfo[500];
							DWORD		HW_Version;
							DWORD		NrOfDmxData;
							BOOL		SignalOutput;
							BYTE		MergeMode;
							WORD		MergeStart;
							WORD		MergeStop;
						};

						typedef bool (__stdcall *DMXForAll_FindInterface)(int maxPortNumber);
						typedef bool (__stdcall *DMXForAll_SetDMX)(int firstChannel, int numBytes, const unsigned char* pData);
						typedef bool (__stdcall *DMXForAll_GetInfo)(VERSION_INFO* vi);
						typedef bool (__stdcall *DMXForAll_GetProductID)(int* pid);
						typedef bool (__stdcall *DMXForAll_GetCOMParam)(int*, DWORD*);

						int _universe;
						DMXForAll_FindInterface PFindInterface;
						DMXForAll_SetDMX PSetDMX;
						DMXForAll_GetInfo PGetInfo;
						DMXForAll_GetProductID PGetProductID;
						DMXForAll_GetCOMParam PGetCOMParam;
						bool _connected;
				};

				class DMX_EXPORTED DMXForAllDeviceClass: public DMXDeviceClass {
					public:
						DMXForAllDeviceClass();
						virtual ~DMXForAllDeviceClass();
						virtual void GetAvailableDevices(std::vector< ref<DMXDevice> >& devs);

					protected:
						ref<DMXForAllDevice> _device;
						HMODULE _module;
				};
			}
		}
	}

	#endif
#endif