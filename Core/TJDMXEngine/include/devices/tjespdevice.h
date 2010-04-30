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
 
 #ifndef _TJESPDEVICE_H
#define _TJESPDEVICE_H

#include "../tjdmxinternal.h"
#include "../tjdmxdevice.h"
#include <TJNP/include/tjsocket.h>

namespace tj {
	namespace dmx {
		namespace devices {
			using namespace tj::shared;
			using namespace tj::dmx;
			using namespace tj::np;

			class DMX_EXPORTED DMXESPDevice: public DMXDevice {
				friend class DMXController;

				public:
					DMXESPDevice();
					virtual ~DMXESPDevice();
					virtual std::wstring GetDeviceID();
					virtual std::wstring GetPort();
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

					ref<Socket> _socket;
					int _universe;
					std::wstring _bcastAddress;
					int _port;
					static const wchar_t* KBroadcastAddress;
					static const int KBroadcastPort;
					unsigned int _outBytes;
					bool _useRLE;
					Timestamp _connected;
			};

			class DMX_EXPORTED DMXESPDeviceClass: public DMXDeviceClass {
				public:
					DMXESPDeviceClass();
					virtual ~DMXESPDeviceClass();
					virtual void GetAvailableDevices(std::vector< ref<DMXDevice> >& devs);
				protected:
					ref<DMXESPDevice> _device;
			};
		}
	}
}

#endif