#include "../../include/devices/tjartnetdevice.h"
#include "../../include/tjdmxcontroller.h"
using namespace tj::dmx::devices;

namespace tj {
	namespace dmx {
		namespace artnet {
			#pragma pack(push,1)

			// ArtDmx packet; see specification
			struct ArtDmx {
				ArtDmx() {
					_id[0] = 'A';
					_id[1] = 'r';
					_id[2] = 't';
					_id[3] = '-';
					_id[4] = 'N';
					_id[5] = 'e';
					_id[6] = 't';
					_id[7] = '\0';
					_op = 0x5000;
					_versionHigh = 0;
					_versionLow = 14;
					_sequence = 0;
					_physical = 0;
					_universe = 0;
					_length = htons(512);
				}

				char _id[8];
				unsigned short _op; // Little endian
				unsigned char _versionHigh;
				unsigned char _versionLow;
				unsigned char _sequence;
				unsigned char _physical;
				unsigned short _universe; // little endian
				unsigned short _length; // big endian (use htons)
				unsigned char _data[512]; // DMX data
			};

			#pragma pack(pop)
		}
	}
}

using namespace tj::dmx::artnet;

const wchar_t* DMXArtNetDevice::KBroadcastAddress = L"2.255.255.255";
const int DMXArtNetDevice::KBroadcastPort = 0x1936;

DMXArtNetDevice::DMXArtNetDevice() {
	_socket = 0;
	_outBytes = 0;
	_inUniverse = 0;
	_outUniverse = 0;
	_universeCount = 1;
	_port = KBroadcastPort;
	_bcastAddress = KBroadcastAddress;
	SetRetransmitEvery(4000); // see the standards
}

DMXArtNetDevice::~DMXArtNetDevice() {
}

std::wstring DMXArtNetDevice::GetDeviceID() {
	return L"protocol.artnet";
}

void DMXArtNetDevice::Save(TiXmlElement* you) {
	SaveAttribute(you, "address", _bcastAddress);
	SaveAttribute(you, "port", _port);
	SaveAttribute(you, "in-universe", _inUniverse);
	SaveAttribute(you, "out-universe", _outUniverse);
	SaveAttribute(you, "universes", _universeCount);
}

void DMXArtNetDevice::Load(TiXmlElement* you) {
	_bcastAddress = LoadAttribute(you, "address", _bcastAddress);
	_port = LoadAttribute(you, "port", _port);
	_inUniverse = LoadAttribute(you, "in-universe", _inUniverse);
	_outUniverse = LoadAttribute(you, "out-universe", _outUniverse);
	_universeCount = LoadAttribute(you, "universes", _universeCount);
}

unsigned int DMXArtNetDevice::GetSupportedUniversesCount() {
	return _universeCount;
}

void DMXArtNetDevice::OnTransmit(ref<DMXController> controller) {
	if(_universeCount>0) {
		NetworkAddress addr(_bcastAddress);
		unsigned int universeCount = controller->GetUniverseCount();
		
		for(int a=0;a<_universeCount;a++) {
			unsigned int inUniverse = _inUniverse + a;
			if(inUniverse<universeCount) {
				int in = inUniverse*512;
				const unsigned char* transmit = controller->GetTransmitBuffer();
				ArtDmx packet;
				packet._universe = (unsigned short)_outUniverse + a;

				for(int b=0;b<512;b++) {
					packet._data[b] = transmit[b+in];
				}

				if(!_socket->SendTo(addr, _port, (const char*)&packet, sizeof(ArtDmx))) {
					Log::Write(L"TJDMX/ArtNetDevice", L"Could not transmit DMX frame");
				}

				_outBytes += (unsigned int)sizeof(ArtDmx);
			}
		}
		Sleep(1000/60); // throttle to 60 packets/sec per universe 
	}
}

std::wstring DMXArtNetDevice::GetDeviceName() const {
	return L"Art-Net";
}

void DMXArtNetDevice::Connect() {
	_socket = GC::Hold(new Socket(AddressFamilyIPv4, TransportProtocolUDP));
	_socket->SetBroadcast(true);
	_socket->SetReuseAddress(true);
	_connected = Timestamp(true);
	_outBytes = 0;
}

std::wstring DMXArtNetDevice::GetPort() {
	if(_universeCount>1) {
		return TL(dmx_artnet_universes)+Stringify(_outUniverse)+L"-"+Stringify(_outUniverse+_universeCount);
	}
	else {
		return TL(dmx_artnet_universe)+Stringify(_outUniverse);
	}
}

#ifdef TJ_DMX_HAS_TJSHAREDUI
	ref<PropertySet> DMXArtNetDevice::GetProperties() {
		ref<PropertySet> prs = GC::Hold(new PropertySet());

		prs->Add(GC::Hold(new PropertySeparator(TL(dmx_artnet_input))));
		prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_artnet_universe_in), this, &_inUniverse, _inUniverse)));
		prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_artnet_universe_count), this, &_universeCount, _universeCount)));

		prs->Add(GC::Hold(new PropertySeparator(TL(dmx_artnet_output))));
		prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_artnet_universe_out), this, &_outUniverse, _outUniverse)));
		prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_artnet_port), this, &_port, _port)));
		prs->Add(GC::Hold(new GenericProperty<std::wstring>(TL(dmx_artnet_address), this, &_bcastAddress, _bcastAddress)));
		return prs;
	}
#endif

std::wstring DMXArtNetDevice::GetDeviceInfo() {
	Timestamp now(true);
	Timestamp difference = now.Difference(_connected);
	float bytesPerSec = float((long double)(_outBytes)/difference.ToMilliSeconds())*1000.0f;
	return L"Data sent: "+ Util::GetSizeString(_outBytes) + L"; average "+Util::GetSizeString((Bytes)bytesPerSec)+L"/s";
}

std::wstring DMXArtNetDevice::GetDeviceSerial() {
	return L"";
}

DMXArtNetDeviceClass::DMXArtNetDeviceClass() {
}

DMXArtNetDeviceClass::~DMXArtNetDeviceClass() {
}

void DMXArtNetDeviceClass::GetAvailableDevices(std::vector< ref<DMXDevice> >& devs) {
	if(!_device) {
		_device = GC::Hold(new DMXArtNetDevice());
	}

	devs.push_back(_device);
}