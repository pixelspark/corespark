#ifndef _TJARTNETDEVICE_H
#define _TJARTNETDEVICE_H

#include "../tjdmxinternal.h"
#include "../tjdmxdevice.h"
#include <TJNP/include/tjsocket.h>

namespace tj {
	namespace dmx {
		namespace devices {
			using namespace tj::shared;
			using namespace tj::dmx;
			using namespace tj::np;

			class DMX_EXPORTED DMXArtNetDevice: public DMXDevice {
				friend class DMXController;

				public:
					DMXArtNetDevice();
					virtual ~DMXArtNetDevice();
					virtual std::wstring GetPort();
					virtual std::wstring GetDeviceID();
					virtual std::wstring GetDeviceInfo();
					virtual std::wstring GetDeviceSerial();
					virtual std::wstring GetDeviceName() const;

					#ifdef TJ_DMX_HAS_TJSHAREDUI
						virtual ref<PropertySet> GetProperties();
					#endif
					virtual unsigned int GetSupportedUniversesCount();

					virtual void Save(TiXmlElement* you);
					virtual void Load(TiXmlElement* you);
					
				protected:
					virtual void Connect();
					virtual void OnTransmit(ref<DMXController> controller);

					ref<Socket> _socket;
					int _inUniverse, _outUniverse, _universeCount;
					std::wstring _bcastAddress;
					int _port;
					static const wchar_t* KBroadcastAddress;
					static const int KBroadcastPort;
					Bytes _outBytes;
					Timestamp _connected;
			};

			class DMX_EXPORTED DMXArtNetDeviceClass: public DMXDeviceClass {
				public:
					DMXArtNetDeviceClass();
					virtual ~DMXArtNetDeviceClass();
					virtual ref< std::vector< ref<DMXDevice> > > GetAvailableDevices();

				protected:
					ref<DMXArtNetDevice> _device;
			};
		}
	}
}

#endif