#include "../../include/devices/tjdevices.h"
#include "../../include/tjdmxcontroller.h"
using namespace tj::dmx::devices;

const wchar_t* DMXESPDevice::KBroadcastAddress = L"255.255.255.255";
const int DMXESPDevice::KBroadcastPort = 3333;

namespace tj {
	namespace dmx {
		namespace esp {
			#pragma pack(push,1)

			struct ESPDataPacket {
				const static unsigned char KDataPlain = 0x1;
				const static unsigned char KDataChannelValue = 0x2;
				const static unsigned char KDataRLE = 0x4;

				ESPDataPacket() {
					_header._universe = 0;
					_header._startDataReader = 0;
					_header._dataSize = htons(512);
					_header._dataType = KDataRLE|KDataPlain;
					_header._head[0] = 'E';
					_header._head[1] = 'S';
					_header._head[2] = 'D';
					_header._head[3] = 'D';
				}

				void SetData(const unsigned char* data, bool useRLE) {
					_header._dataType = useRLE?KDataRLE:KDataPlain;

					if(useRLE) {
						_header._dataSize = 0;

						for(int a=0;a<512;a++) {
							unsigned char current = data[a];
							
							// try to do RLE
							if(a<510 && data[a+1]==current && data[a+2]==current) {
								_data[_header._dataSize] = 0xFE;
								_header._dataSize++;

								unsigned char num = 0;
								for(;a<512 && num<255;++a) {
									if(data[a+1]==current) {
										++num;
									}
									else {
										--a;
										break;
									}
								}
								
								_data[_header._dataSize] = num;
								_header._dataSize++;

								_data[_header._dataSize] = current;
								_header._dataSize++;
							}
							else {
								if(current==0xFE||current==0xFD) { // escape byte for 0xFE
									_data[_header._dataSize] = 0xFD;
									_header._dataSize++;
								}
								_data[_header._dataSize] = current;
								_header._dataSize++;
							}
						}

						_header._dataSize = htons(_header._dataSize);
					}
					else {
						_header._dataSize = htons(512);
						for(int a=0;a<512;a++) {
							_data[a] = data[a];
						}
					}
				}

				size_t GetSize() {
					return sizeof(Header)+ntohs(_header._dataSize)*sizeof(char);
				}
				
				struct Header {
					char _head[4];
					unsigned char _universe;
					unsigned char _startDataReader;
					unsigned char _dataType;
					unsigned short _dataSize; // in network byte order!
				} _header;
				unsigned char _data[512];
			};
			
			#pragma pack(pop)
		}
	}
}

using namespace tj::dmx::esp;

DMXESPDevice::DMXESPDevice() {
	_universe = 0;
	_outBytes = 0;
	_port = KBroadcastPort;
	_bcastAddress = KBroadcastAddress;
	SetRetransmitEvery(1000); // just following the standards here
	_useRLE = false;
}

DMXESPDevice::~DMXESPDevice() {
}

void DMXESPDevice::Save(TiXmlElement* you) {
	SaveAttribute(you, "address", _bcastAddress);
	SaveAttribute(you, "port", _port);
	SaveAttribute(you, "universe", _universe);
}

void DMXESPDevice::Load(TiXmlElement* you) {
	_bcastAddress = LoadAttribute(you, "address", _bcastAddress);
	_port = LoadAttribute(you, "port", _port);
	_universe = LoadAttribute(you, "universe", _universe);
}

std::wstring DMXESPDevice::GetDeviceID() {
	return L"protocol.esp";
}

void DMXESPDevice::OnTransmit(ref<DMXController> controller) {
	NetworkAddress addr(_bcastAddress, false);
	
	if(_socket) {
		ESPDataPacket esp;
		const unsigned char* transmit = controller->GetTransmitBuffer();
		esp.SetData(transmit, _useRLE);

		int ob = esp.GetSize();
		if(!_socket->SendTo(addr, _port, (const char*)&esp, (int)ob)) {
			Log::Write(L"TJDMX/ESPDevice", L"Could not transmit DMX frame");
		}
		_outBytes += (unsigned int)ob;
	}
	
	// TODO: this throttles the whole transmit thread...
	Sleep(1000/60); // throttle to 60 packets/sec
}

std::wstring DMXESPDevice::GetDeviceName() const {
	return L"ESP (Ent-Tec Ethernet Protocol)";
}

unsigned int DMXESPDevice::GetSupportedUniversesCount() {
	return 1;
}

void DMXESPDevice::Connect() {
	_socket = GC::Hold(new Socket(AddressFamilyIPv4, TransportProtocolUDP));
	_socket->SetBroadcast(true);
	_socket->SetReuseAddress(true);
	_connected = Timestamp(true);
	_outBytes = 0;
}

std::wstring DMXESPDevice::GetPort() {
	return Stringify(_universe);
}

#ifdef TJ_DMX_HAS_TJSHAREDUI
	ref<PropertySet> DMXESPDevice::GetProperties() {
		ref<PropertySet> prs = GC::Hold(new PropertySet());
		prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_esp_universe), this, &_universe, _universe)));
		prs->Add(GC::Hold(new GenericProperty<int>(TL(dmx_esp_port), this, &_port, _port)));
		prs->Add(GC::Hold(new GenericProperty<std::wstring>(TL(dmx_esp_address), this, &_bcastAddress, _bcastAddress)));
		prs->Add(GC::Hold(new GenericProperty<bool>(TL(dmx_esp_use_rle), this, &_useRLE, _useRLE)));
		return prs;
	}
#endif

std::wstring DMXESPDevice::GetDeviceInfo() {
	Timestamp now(true);
	Timestamp difference = now.Difference(_connected);
	float bytesPerSec = float((long double)(_outBytes)/difference.ToMilliSeconds())*1000.0f;
	return L"Data sent: "+ Util::GetSizeString(_outBytes) + L"; average "+Util::GetSizeString((Bytes)bytesPerSec)+L"/s";
}

std::wstring DMXESPDevice::GetDeviceSerial() {
	return L"";
}

DMXESPDeviceClass::DMXESPDeviceClass() {
}

DMXESPDeviceClass::~DMXESPDeviceClass() {
}

ref< std::vector< ref<DMXDevice> > > DMXESPDeviceClass::GetAvailableDevices() {
	if(!_device) {
		_device = GC::Hold(new DMXESPDevice());
	}

	ref< std::vector< ref<DMXDevice> > > devs = GC::Hold(new std::vector< ref<DMXDevice> >());
	devs->push_back(_device);
	return devs;
}