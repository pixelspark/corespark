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