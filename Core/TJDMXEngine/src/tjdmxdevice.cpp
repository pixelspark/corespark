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
 
 #include "../include/tjdmxdevice.h"
#include "../include/tjdmxcontroller.h"
using namespace tj::dmx;

DMXDevice::DMXDevice(): _running(false) {
}

DMXDevice::~DMXDevice() {
	Stop();
}

void DMXDevice::SetRetransmitEvery(Time ms) {
	_retransmitTime = ms;
}

void DMXDevice::Save(TiXmlElement* you) {
}

void DMXDevice::Load(TiXmlElement* you) {
}

void DMXDevice::Stop() {
	ThreadLock lock(&_lock);
	_running = false;
	_update.Signal();
}

void DMXDevice::Run() {
	_running = true;
	Log::Write(L"TJDMX/DMXDevice", L"Started transmit thread");
	SetName(std::wstring(L"DMXDevice thread: ")+GetDeviceName());

	Connect();

	while(_update.Wait()) {
		_update.Reset();
		if(_running) {
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
	_update.Signal();
}

CriticalSection* DMXDevice::GetLock() {
	return &_lock;
}

DMXDeviceClass::~DMXDeviceClass() {
}