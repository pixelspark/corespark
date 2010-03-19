#include "../../include/devices/tjdevices.h"
#include "../../include/tjdmxcontroller.h"
using namespace tj::dmx::devices;
using namespace tj::dmx;

#ifdef TJ_OS_WIN
#define READ_ONE_BYTE(handle,byte,bytes_read,overlapped_struct) ReadFile(handle,byte,1,bytes_read,overlapped_struct)

DMXEnttecProDevice::DMXEnttecProDevice(): _universe(0) {
	_port = L"";
	_file = INVALID_HANDLE_VALUE;
	_transmitBuffer = new unsigned char[513];
}

DMXEnttecProDevice::~DMXEnttecProDevice() {
	int size;
	int res = SendData(8,(unsigned char *)&size,2);
	CloseHandle(_file);
	delete[] _transmitBuffer;
}

void DMXEnttecProDevice::OnTransmit(ref<DMXController> controller) {
	if(_universe < int(controller->GetUniverseCount())) {
		const unsigned char* data = controller->GetTransmitBuffer();
		_transmitBuffer[0] = 0x0; // Start code
		unsigned int offset = _universe * 512;

		for(int a=0;a<512;a++) {
			_transmitBuffer[a+1] = data[offset+a];
		}

		if(SendData(SEND_DMX,(unsigned char const*)_transmitBuffer,513)!=0) {
			Sleep(1000);
			Connect(); // TODO set silent connect
		}		
	}
}

void DMXEnttecProDevice::Load(TiXmlElement* you) {
	_universe = LoadAttributeSmall<int>(you, "universe", _universe);
}

void DMXEnttecProDevice::Save(TiXmlElement* you) {
	SaveAttributeSmall(you, "universe", _universe);
}

std::wstring DMXEnttecProDevice::GetDeviceID() {
	return L"device.enttec.usbprodmx";
}

std::wstring DMXEnttecProDevice::GetDeviceName() const {
	return L"Ent-Tec USB Pro DMX";
}

#ifdef TJ_DMX_HAS_TJSHAREDUI
	ref<PropertySet> DMXEnttecProDevice::GetProperties() {
		ref<PropertySet> prs = GC::Hold(new PropertySet());
		prs->Add(GC::Hold(new PropertySeparator(TL(dmx_device_universes))));
		prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_device_universe_in), this, &_universe, _universe)));
		return prs;
	}
#endif

void DMXEnttecProDevice::Connect() {
	_port = FindDevice() +L":"; 
	if(_port.length()==1) {
		_file = INVALID_HANDLE_VALUE;
		return;
	}
	_file = CreateFile(_port.c_str(), GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
	if(_file==INVALID_HANDLE_VALUE) {
		Log::Write(L"TJDMX/DMXEnttecProDevice", Stringify(_port) + std::wstring(L"Could not open DMX file handle; is your device connected?"));
		_file = INVALID_HANDLE_VALUE;
		return;
	}

	SetCommParameters();
	_info = AskDeviceInfo();
	_serial = AskDeviceSerial();
}

std::wstring DMXEnttecProDevice::GetDeviceSerial() {
	return _serial;
}

unsigned int DMXEnttecProDevice::GetSupportedUniversesCount() {
	return 1;
}

std::wstring DMXEnttecProDevice::AskDeviceSerial() {
	ThreadLock lock(GetLock());
	int size;
	int res = SendData(GET_WIDGET_SERIAL,(unsigned char *)&size,2);
	if(res < 0) {
		return L"";
	}

	int serial = 0;
	res = ReceiveData(GET_WIDGET_SERIAL_REPLY,(unsigned char*)&serial,4);
	if(res < 0) {
		return L"[Unknown]";
	}

	return Stringify((unsigned int)serial);
}

std::wstring DMXEnttecProDevice::GetPort() {
	return _port;
}

std:: wstring DMXEnttecProDevice::FindDevice() {
	unsigned char DeviceName[256];
	HKEY hKey;
	std::wstring DeviceNameW;
	DWORD DeviceNameLen, KeyNameLen;
	wchar_t KeyName[256];
	int i;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
		return  L"";
	}

	i = 0;
	while(i < 50) {
		DeviceNameLen = 80;
		KeyNameLen = 100;
		if (RegEnumValue(hKey,i,KeyName,&KeyNameLen,NULL,NULL,DeviceName,&DeviceNameLen) != ERROR_SUCCESS) break;
		
		DeviceNameW = std::wstring((const wchar_t*)DeviceName);

		if (!wcsncmp(DeviceNameW.c_str(),L"\\Device\\VCP0",12)) {
			// we found a serial COM device, COM port "i"
			break;
		}

		DeviceName[0] = 0;                     
		i++;
	}
	RegCloseKey(hKey);

	if (i == 50) {
		return L"";
	}

	//Log::Write(L"TJDMX/TransmitThread",std::wstring(L"Found controller device: ")+DeviceNameW);
	return DeviceNameW;
}

std::wstring DMXEnttecProDevice::GetDeviceInfo() {
	return _info;
}

std::wstring DMXEnttecProDevice::AskDeviceInfo() {
	ThreadLock lock(&_lock);
	int size;
	int res = SendData(GET_WIDGET_PARAMS,(unsigned char *)&size,2);
	if(res < 0) {
		return L"";
	}

	res = ReceiveData(GET_WIDGET_PARAMS_REPLY,(unsigned char *)&_parameters,sizeof(enttec::DMXUSBPROParamsType));
	if(res < 0) {
		return L"";
	}

	return std::wstring(L"Ent-tec DMX USB Pro v")+Stringify(_parameters.FirmwareMSB)+L":"+Stringify(_parameters.FirmwareLSB);
}

void DMXEnttecProDevice::SetCommParameters() {
	ThreadLock lock(&_lock);
	// SetCommState & Timeouts
	DCB dcb;
	GetCommState(_file, &dcb);

	dcb.fBinary = TRUE; /* binary mode, no EOF 
						check */
	dcb.fErrorChar = FALSE; /* disable error replacement */
	dcb.fAbortOnError = FALSE;

	/* Set the baud rate */
	dcb.BaudRate = 57600;

	/* Set the data characteristics */
	dcb.ByteSize = 8; /* 8 data bits */
	dcb.StopBits = ONESTOPBIT; /* 1 stop bit */
	dcb.fParity = NOPARITY; /* no parity */
	dcb.Parity = 0;

	/* disable all flow control stuff */
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fInX = FALSE;
	dcb.fOutX = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	SetCommState(_file, &dcb);

    COMMTIMEOUTS timeouts;
	GetCommTimeouts(_file, &timeouts);
	/* set timimg values */
	timeouts.ReadIntervalTimeout = 500;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.ReadTotalTimeoutConstant = 500;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 500;
	SetCommTimeouts(_file, &timeouts);
}


// send a packet, overlapped code has not been implemented yet
int DMXEnttecProDevice::SendData(int label, unsigned char const* data, int length) {
	ThreadLock lock(&_lock);

	BOOL res = false;
	DWORD bytes_written = 0;
	HANDLE event = NULL;

	unsigned char header[4];
	header[0] = 0x7E;
	header[1] = label;
	header[2] = length & 0xFF;
	header[3] = length >> 8;

	res = WriteFile(_file,(unsigned char *)header,4,&bytes_written,NULL);
	if (!res || (bytes_written != 4)) return -1;

	res = WriteFile(_file,data,length,&bytes_written,NULL);
	if (!res || (bytes_written != length)) return -1;

	unsigned char end_code = 0xE7;
	res = WriteFile(_file,(unsigned char *)&end_code,1,&bytes_written,NULL);
	if (!res || (bytes_written != 1)) return -1;

	return 0;
}

// read a packet, overlapped code has not been implemented yet
int DMXEnttecProDevice::ReceiveData(int label, unsigned char *data, unsigned int expected_length) {
	ThreadLock lock(&_lock);

	if(_file<=0) {
		Connect();	
		if(_file<=0) return 0;
	}
	
	BOOL res = false;
	DWORD length = 0;
	DWORD bytes_read = 0;
	unsigned char byte = 0;
	HANDLE event = NULL;

	while (byte != label) {
		while (byte != 0x7E) {
			res = READ_ONE_BYTE(_file,(unsigned char *)&byte,&bytes_read,NULL);
			if (!res || (bytes_read != 1)) return -1;
		}
		if (byte != 0x7E) continue;

		res = READ_ONE_BYTE(_file,(unsigned char *)&byte,&bytes_read,NULL);
		if (!res || (bytes_read != 1)) return -1;
	}

	res = READ_ONE_BYTE(_file,(unsigned char *)&byte,&bytes_read,NULL);
	if (!res || (bytes_read != 1)) return -1;
	length = byte;


	res = READ_ONE_BYTE(_file,(unsigned char *)&byte,&bytes_read,NULL);
	if (!res || (bytes_read != 1)) return -1;
	length += byte<<8;

	if (length > 600)
		return -1;
	if (length < expected_length)
		return -1;

	char buffer[600];
	res = ReadFile(_file,buffer,length,&bytes_read,NULL);
	if (!res || (bytes_read != length)) return -1;

	res = READ_ONE_BYTE(_file,(unsigned char *)&byte,&bytes_read,NULL);
	if (!res || (bytes_read != 1)) return -1;
	if (byte != 0xE7) return -1;
	memcpy(data,buffer,expected_length);
	return expected_length;
}

DMXEnttecProDeviceClass::DMXEnttecProDeviceClass() {
}

DMXEnttecProDeviceClass::~DMXEnttecProDeviceClass() {
}

void DMXEnttecProDeviceClass::GetAvailableDevices(std::vector< ref<DMXDevice> >& devs) {
	if(!_device) {
		_device = GC::Hold(new DMXEnttecProDevice());
	}

	devs.push_back(_device);
}

#endif