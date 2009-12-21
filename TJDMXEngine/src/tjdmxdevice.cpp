#include "../include/tjdmxdevice.h"
#include "../include/tjdmxcontroller.h"
using namespace tj::dmx;

DMXDevice::DMXDevice() {
}

DMXDevice::~DMXDevice() {
	_end.Signal();
}

void DMXDevice::SetRetransmitEvery(Time ms) {
	_retransmitTime = ms;
}

void DMXDevice::Save(TiXmlElement* you) {
}

void DMXDevice::Load(TiXmlElement* you) {
}

void DMXDevice::Stop() {
	Log::Write(L"TJDMX/DMXDevice", L"Stop device "+GetDeviceName());
	_end.Signal();
}

void DMXDevice::Run() {
	Log::Write(L"TJDMX/DMXDevice", L"Started transmit thread");
	SetName(std::wstring(L"DMXDevice thread: ")+GetDeviceName());

	Connect();

	Wait wait; wait[_transmit][_end];

	while(true) {
		int r = wait.ForAny(_retransmitTime);
		if(r!=1) {
			_transmit.Reset();
			ThreadLock lock(&_lock);
			ref<DMXController> controller = _controller;
			if(controller) {
				controller->Process(); // make sure we get the latest DMX data
				OnTransmit(controller);	
			}
			else {
				Log::Write(L"TJDMX/DMXDevice", L"No controller to transmit data from!");
			}
		}
		else {
			_end.Reset();
			Log::Write(L"TJDMX/DMXDevice", L"Thread end");
			return;
		}
	}
}

void DMXDevice::SetController(ref<DMXController> c) {
	ThreadLock lock(&_lock);
	_controller = c;
}

void DMXDevice::Transmit() {
	_transmit.Signal();
}

CriticalSection* DMXDevice::GetLock() {
	return &_lock;
}

DMXDeviceClass::~DMXDeviceClass() {
}