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