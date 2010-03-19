#include "../../include/devices/tjdevices.h"
#ifdef TJ_OS_WIN
	#include "../../include/tjdmxcontroller.h"
	using namespace tj::dmx::devices;
	using namespace tj::dmx;

	DMXForAllDevice::DMXForAllDevice(HMODULE module): _universe(0), _connected(false) {
		PFindInterface = (DMXForAll_FindInterface)GetProcAddress(module, "__FindInterface");
		PSetDMX = (DMXForAll_SetDMX)GetProcAddress(module, "__SetDmx");
		PGetInfo = (DMXForAll_GetInfo)GetProcAddress(module, "__GetInfo");
		PGetProductID = (DMXForAll_GetProductID)GetProcAddress(module, "__GetProductID");
		PGetCOMParam = (DMXForAll_GetCOMParam)GetProcAddress(module, "__GetComParameters");
	}

	DMXForAllDevice::~DMXForAllDevice() {
	}

	unsigned int DMXForAllDevice::GetSupportedUniversesCount() {
		return 1;
	}

	std::wstring DMXForAllDevice::GetPort() {
		int port = 0;
		unsigned long baudRate = 0;

		if(PGetCOMParam!=0 && PGetCOMParam(&port, &baudRate)) {
			return Stringify(port);
		}

		return L"";
	}

	std::wstring DMXForAllDevice::GetDeviceID() {
		return L"device.dmx4all";
	}

	std::wstring DMXForAllDevice::GetDeviceInfo() {
		if(_connected) {
			// Get product id
			int pid = 0;
			if(PGetProductID==0 || !PGetProductID(&pid)) {
				Log::Write(L"TJDMX/DMX4AllDevice", L"Could not get product ID, GetProductID==0 or returned false");
			}

			VERSION_INFO vi;
			if(PGetInfo!=0) {
				if(PGetInfo(&vi)) {
					return L"Type="+Stringify(pid)+L" Version="+Stringify(vi.HW_Version)+L" Output="+(vi.SignalOutput ? L"Yes": L"No");
				}
				else {
					Log::Write(L"TJDMX/DMX4AllDevice",L"Could not get info (PGetInfo returned false)!");
				}
			}
			else {
				Log::Write(L"TJDMX/DMX4AllDevice",L"Could not get info (PGetInfo==0)!");
			}
		}

		return L"";
	}

	std::wstring DMXForAllDevice::GetDeviceSerial() {
		return L"";
	}

	std::wstring DMXForAllDevice::GetDeviceName() const {
		return L"DMX4All USB-DMX";
	}

	#ifdef TJ_DMX_HAS_TJSHAREDUI
		ref<PropertySet> DMXForAllDevice::GetProperties() {
			ref<PropertySet> prs = GC::Hold(new PropertySet());
			prs->Add(GC::Hold(new PropertySeparator(TL(dmx_device_universes))));
			prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_device_universe_in), this, &_universe, _universe)));
			return prs;
		}
	#endif

	void DMXForAllDevice::Save(TiXmlElement* you) {
		SaveAttributeSmall(you, "universe", _universe);
	}

	void DMXForAllDevice::Load(TiXmlElement* you) {
		_universe = LoadAttributeSmall<int>(you, "universe", _universe);
	}

	void DMXForAllDevice::Connect() {
		if(PFindInterface!=0 && PSetDMX!=0) {
			if(PFindInterface(9)) {
				_connected = true;
			}
			else {
				_connected = false;
				Log::Write(L"TJDMX/DMX4AllDevice", L"No device found, cannot connect!");
			}
		}
	}

	void DMXForAllDevice::OnTransmit(ref<DMXController> controller) {
		if(_connected) {
			if(PSetDMX==0) {
				Log::Write(L"TJDMX/DMX4AllDevice", L"Cannot transmit, PSetDMX==0!");
			}
			else {
				const unsigned char* data = controller->GetTransmitBuffer();
				if((unsigned int)(_universe) < controller->GetUniverseCount()) {
					const unsigned char* universeData = &data[_universe * 512];

					// Read the ports to ensure a USB connection and prevent the device from 
					// entering 'standalone mode'.
					if(!PSetDMX(1, 512, universeData)) {
						Log::Write(L"TJDMX/DMX4AllDevice", L"Could not send DMX!");
					}
				}
			}
		}
	}

	/** DMXForAllDeviceClass **/
	DMXForAllDeviceClass::DMXForAllDeviceClass() {
		_module = LoadLibrary(L"drivers/dmx4all.dll");
		if(_module!=NULL) {
			_device = GC::Hold(new DMXForAllDevice(_module));
		}
		else {
			Log::Write(L"TJDMX/DMXForAllDevice", L"Could not load dmx4all.dll; is it installed correctly? Also, check if the FTDI drivers are installed correctly (ftd2xx.dll)");
		}
	}

	DMXForAllDeviceClass::~DMXForAllDeviceClass() {
	}

	void DMXForAllDeviceClass::GetAvailableDevices(std::vector< ref<DMXDevice> >& devs) {
		if(_device) {
			devs.push_back(_device);
		}
	}
#endif