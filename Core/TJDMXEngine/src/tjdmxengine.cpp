#include "../include/tjdmxinternal.h"
#include "../include/tjdmxcontroller.h"
#include "../include/devices/tjdevices.h"
using namespace tj::dmx;
using namespace tj::dmx::devices;

ref<DMXController> DMXEngine::_instance;

strong<DMXController> DMXEngine::GetController() {
	if(!_instance) {
		_instance = GC::Hold(new DMXController());
		_instance->AddDeviceClass(GC::Hold(new DMXESPDeviceClass()));
		_instance->AddDeviceClass(GC::Hold(new DMXEnttecProDeviceClass()));
		_instance->AddDeviceClass(GC::Hold(new DMXArtNetDeviceClass()));
		_instance->AddDeviceClass(GC::Hold(new DMXLanboxDeviceClass()));

		#ifdef TJ_OS_WIN
			_instance->AddDeviceClass(GC::Hold(new DMXSoundLightDeviceClass()));
			_instance->AddDeviceClass(GC::Hold(new DMXForAllDeviceClass()));
		#endif
	}

	return _instance;
}