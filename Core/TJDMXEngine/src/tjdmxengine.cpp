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
		_instance->AddDeviceClass(GC::Hold(new DMXArtNetDeviceClass()));
		_instance->AddDeviceClass(GC::Hold(new DMXLanboxDeviceClass()));

		#ifdef TJ_OS_WIN
			_instance->AddDeviceClass(GC::Hold(new DMXEnttecProDeviceClass()));
			_instance->AddDeviceClass(GC::Hold(new DMXSoundLightDeviceClass()));
			_instance->AddDeviceClass(GC::Hold(new DMXForAllDeviceClass()));
		#endif
	}

	return _instance;
}