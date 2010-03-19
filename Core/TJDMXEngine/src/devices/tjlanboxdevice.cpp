#include "../../include/devices/tjlanboxdevice.h"
#include "../../include/tjdmxcontroller.h"
using namespace tj::dmx::devices;
using namespace tj::np;

/** 
Implentation of the DMX Lanbox LCE protocol (UDP) 
See www.lanbox.com for the devices that support this protocol. This implementation is based on the 
example implementation (included with the LCEditPlus application) by Kurt Sterckx (LANBOX_DMXOUT.cpp).
**/

const std::wstring DMXLanboxDevice::KDefaultAddress = L"192.168.1.77";
const int DMXLanboxDevice::KDefaultPort = 4777;

#define HighByte(a) ((unsigned char) ((a) >> 8))
#define LowByte(a) ((unsigned char) ((a) & 0xFF))

DMXLanboxDevice::DMXLanboxDevice(): _universe(0), _outBytes(0), _port(KDefaultPort), _address(KDefaultAddress), _sequence(0) {
	SetRetransmitEvery(1000); // just following the standards here
}

DMXLanboxDevice::~DMXLanboxDevice() {
}

std::wstring DMXLanboxDevice::GetDeviceID() {
	return L"protocol.lanbox";
}

std::wstring DMXLanboxDevice::GetPort() {
	return Stringify(_universe);
}

std::wstring DMXLanboxDevice::GetDeviceInfo() {
	Timestamp now(true);
	Timestamp difference = now.Difference(_connected);
	float bytesPerSec = float((long double)(_outBytes)/difference.ToMilliSeconds())*1000.0f;
	return L"Data sent: "+ Util::GetSizeString(_outBytes) + L"; average "+Util::GetSizeString((Bytes)bytesPerSec)+L"/s";
}

std::wstring DMXLanboxDevice::GetDeviceSerial() {
	return L"";
}

std::wstring DMXLanboxDevice::GetDeviceName() const {
	return L"LanBox-LCE Protocol (over UDP)";
}

#ifdef TJ_DMX_HAS_TJSHAREDUI
	ref<PropertySet> DMXLanboxDevice::GetProperties() {
		ref<PropertySet> prs = GC::Hold(new PropertySet());
		prs->Add(GC::Hold(new GenericProperty<unsigned int>(TL(dmx_lanbox_universe_in), this, &_universe, _universe)));
		prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_lanbox_port), this, &_port, _port)));
		prs->Add(GC::Hold(new GenericProperty<std::wstring>(TL(dmx_lanbox_address), this, &_address, _address)));
		return prs;
	}
#endif

unsigned int DMXLanboxDevice::GetSupportedUniversesCount() {
	return 1;
}

void DMXLanboxDevice::Save(TiXmlElement* you) {
	ThreadLock lock(&_lock);
	SaveAttribute(you, "address", _address);
	SaveAttribute(you, "port", _port);
	SaveAttribute(you, "universe", _universe);
}

void DMXLanboxDevice::Load(TiXmlElement* you) {
	ThreadLock lock(&_lock);
	_address = LoadAttribute(you, "address", _address);
	_port = LoadAttribute(you, "port", _port);
	_universe = LoadAttribute(you, "universe", _universe);
}

void DMXLanboxDevice::Connect() {
	ThreadLock lock(&_lock);
	_socket = GC::Hold(new Socket(AddressFamilyIPv4, TransportProtocolUDP));
	_sequence = 0;
	_connected.Now();
}

void DMXLanboxDevice::OnTransmit(ref<DMXController> controller) {
	const static unsigned int KDataLength = 512;
	const static unsigned int KFirstChannel = 1;
	const static unsigned int KPacketSize = 4 + 6 + 512; // packet header (4) + message header (6) + DMX values

	ThreadLock lock(&_lock);
	unsigned int universeCount = controller->GetUniverseCount();

	if(_socket->IsValid() && _universe < universeCount) {
		NetworkAddress address(_address, false);

		++_sequence;
		unsigned char buffer[KPacketSize]; 

		// Packet Header
		buffer[0] = 0xC0;
		buffer[1] = 0xB7;
		buffer[2] = HighByte(_sequence);
		buffer[3] = LowByte(_sequence);

		// Message Header
		buffer[4] = 0xCA;
		buffer[5] = 254;
		buffer[6] = HighByte(6 + KDataLength);
		buffer[7] = LowByte(6 + KDataLength);
		buffer[8] = HighByte(KFirstChannel);
		buffer[9] = LowByte(KFirstChannel);

		const unsigned char* data = controller->GetTransmitBuffer();
		
		for(unsigned int a = 0; a < 512; ++a) {
			buffer[a+10] = data[a + (_universe*512)];
		}

		if(!_socket->SendTo(address, _port, (const char*)buffer, KPacketSize)) {
			Log::Write(L"TJDMX/DMXLanboxDevice" , L"Could not send DMX packet to LanBox");
		}
		_outBytes += KPacketSize;
	}
}

/** DMXLanboxDeviceClass **/
DMXLanboxDeviceClass::DMXLanboxDeviceClass() {
}

DMXLanboxDeviceClass::~DMXLanboxDeviceClass() {
}

void DMXLanboxDeviceClass::GetAvailableDevices(std::vector< ref<DMXDevice> >& devs) {
	if(!_device) {
		_device = GC::Hold(new DMXLanboxDevice());
	}

	devs.push_back(_device);
}
