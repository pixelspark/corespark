#include "../../include/devices/tjdevices.h"

#ifdef TJ_OS_WIN
	#include "../../include/tjdmxcontroller.h"
	#include "tjsoundlight.h"
	using namespace tj::dmx::devices;
	using namespace tj::dmx::soundlight;

	/** Note that in the future, we can support multiple devices! From the site, 
	http://www.pcdmx512.com/ger/hardware/usbdmx1/dllinfo.htm:

	You can use up to 10 USB interfaces at the same time.
	To do this, just add a value in the <command> parameter :
	- add 100 (DHC_USBDMX11) if you want to use the interface #2
	- add 200 (DHC_USBDMX12) if you want to use the interface #3 ...
	Example:       DasHardCommand( DHC_USBDMX11+DHC_OPEN  ,   0  ,  0 )   open the the interface #2

	*/

	DMXSoundLightDevice::DMXSoundLightDevice(HMODULE module): _universe(0) {
		// try to open the library
		_proc = (HardDllCommand)GetProcAddress(module, "DasHardCommand");
		if(_proc==0) {
			Log::Write(L"TJDMX/DMXSoundLightDevice", L"Could not find DasHardCommand procedure in driver; is the correct dashard.dll installed?");
		}
	}

	DMXSoundLightDevice::~DMXSoundLightDevice() {
		if(_proc!=0) {
			_proc(SoundLightConstants::DHC_OPEN, 0,0);
		}
	}

	unsigned int DMXSoundLightDevice::GetSupportedUniversesCount() {
		return 1;
	}

	std::wstring DMXSoundLightDevice::GetPort() {
		return L"";
	}

	std::wstring DMXSoundLightDevice::GetDeviceID() {
		return L"device.soundlight";
	}

	std::wstring DMXSoundLightDevice::GetDeviceInfo() {
		if(_proc!=0) {
			int version = _proc(SoundLightConstants::DHC_VERSION,0,0);
			return L"Firmware version: "+Stringify(version);
		}
		return L"Not connected";
	}

	std::wstring DMXSoundLightDevice::GetDeviceSerial() {
		return L"";
	}

	std::wstring DMXSoundLightDevice::GetDeviceName() const {
		return L"SoundLight USB-DMX";
	}

	#ifdef TJ_DMX_HAS_TJSHAREDUI
		ref<PropertySet> DMXSoundLightDevice::GetProperties() {
			ref<PropertySet> prs = GC::Hold(new PropertySet());
			prs->Add(GC::Hold(new PropertySeparator(TL(dmx_device_universes))));
			prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_device_universe_in), this, &_universe, _universe)));
			return prs;
		}
	#endif

	void DMXSoundLightDevice::Save(TiXmlElement* you) {
		SaveAttributeSmall(you, "universe", _universe);
	}

	void DMXSoundLightDevice::Load(TiXmlElement* you) {
		_universe = LoadAttributeSmall<int>(you, "universe", _universe);
	}

	void DMXSoundLightDevice::Connect() {
		if(_proc!=0) {
			int ret = _proc(SoundLightConstants::DHC_OPEN,0,0);
			switch(ret) {
				case SoundLightConstants::DeviceUsbDMX1:
					Log::Write(L"TJDMX/DMXSoundLightDevice", L"USBDMX1 widget connected");
					break;

				case SoundLightConstants::DeviceUsbDMX2:
					Log::Write(L"TJDMX/DMXSoundLightDevice", L"USBDMX2 widget connected");
					break;

				default:
					Log::Write(L"TJDMX/DMXSoundLightDevice", L"USB widget not plugged in; could not connect!");
			}

			if(ret>0) {
				// Clear DMX levels to be sure
				_proc(SoundLightConstants::DHC_DMXOUTOFF,0,0);
			}
		}
	}

	void DMXSoundLightDevice::OnTransmit(ref<DMXController> controller) {
		if(_proc!=0) {
			const unsigned char* data = controller->GetTransmitBuffer();
			if((unsigned int)(_universe) < controller->GetUniverseCount()) {
				const unsigned char* universeData = &data[_universe * 512];

				// Read the ports to ensure a USB connection and prevent the device from 
				// entering 'standalone mode'.
				int ports = _proc(SoundLightConstants::DHC_PORTREAD,0,0);
				if(_proc(SoundLightConstants::DHC_DMXOUT, 512, (unsigned char*)universeData)<0) {
					// An error occurred, reconnect
					_proc(SoundLightConstants::DHC_CLOSE,0,0);
					Connect();
				}
			}
		}
	}

	/** DMXSoundLightDeviceClass **/
	DMXSoundLightDeviceClass::DMXSoundLightDeviceClass() {
		_module = LoadLibrary(L"drivers/dashard.dll");
		if(_module!=NULL) {
			_device = GC::Hold(new DMXSoundLightDevice(_module));
		}
		else {
			Log::Write(L"TJDMX/DMXSoundLightDevice", L"Could not load dashard.dll; is it installed correctly?");
		}
	}

	DMXSoundLightDeviceClass::~DMXSoundLightDeviceClass() {
		
	}

	void DMXSoundLightDeviceClass::GetAvailableDevices(std::vector< ref<DMXDevice> >& devs) {
		if(_device) {
			devs.push_back(_device);
		}
	}
#endif