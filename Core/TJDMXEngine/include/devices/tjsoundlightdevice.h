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
#ifndef _TJSOUNDLIGHTDEVICE_H
#define _TJSOUNDLIGHTDEVICE_H

#include "../tjdmxinternal.h"
#include "../tjdmxdevice.h"

namespace tj {
	namespace dmx {
		namespace devices {
			using namespace tj::shared;
			using namespace tj::dmx;

			class DMXSoundLightDevice: public DMXDevice {
				friend class DMXController;

				public:
					DMXSoundLightDevice(HMODULE module);
					virtual ~DMXSoundLightDevice();
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

					typedef int (*HardDllCommand)(int command, int param, unsigned char *bloc);
					HardDllCommand _proc;
					int _universe;
			};

			class DMXSoundLightDeviceClass: public DMXDeviceClass {
				public:
					DMXSoundLightDeviceClass();
					virtual ~DMXSoundLightDeviceClass();
					virtual void GetAvailableDevices(std::vector< ref<DMXDevice> >& devs);

				protected:
					ref<DMXSoundLightDevice> _device;
					HMODULE _module;
			};
		}
	}
}

#endif
#endif