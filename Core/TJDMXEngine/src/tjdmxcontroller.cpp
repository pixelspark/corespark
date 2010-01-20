#include "../include/tjdmxcontroller.h"
#include "../include/tjdmxmacro.h"
#include <algorithm>
using namespace tj::dmx;

class GrandMasterMacro: public DMXMacro {
	public:
		GrandMasterMacro(DMXController* controller, DMXSource src, bool invert): _controller(controller), _src(src), _invert(invert) {}
		virtual ~GrandMasterMacro() {}
		
		virtual void Set(float value) {
			if(_invert) value = 1.0f-value;
			_controller->SetGrandMaster(int(value*255.0f), _src);
		}
		
		virtual float Get() const {
			float v = float(_controller->GetGrandMaster(_src))/255.0f;
			return _invert?(1.0f-v):v;
		}

		virtual std::wstring GetAddress() const { 
			return (_invert?L"-":L"")+std::wstring(L"gm");
		}

		virtual float GetResult() const {
			float v = _controller->GetGrandMasterValue();
			return _invert?(1.0f-v):v;
		}

		virtual DMXMacro::DMXMacroType GetType() const {
			return DMXMacroTypeMaster;
		}

		virtual float GetResultCached() const {
			float v = _controller->GetGrandMasterValue();
			return _invert?(1.0f-v):v;
		}

	protected:
		DMXController* _controller;
		DMXSource _src;
		bool _invert;
};

class SequenceMasterMacro: public DMXMacro {
	public:
		SequenceMasterMacro(DMXController* controller, DMXSource src, bool invert): _controller(controller), _src(src), _invert(invert) {}
		virtual ~SequenceMasterMacro() {}
		
		virtual void Set(float value) {
			if(_invert) value = 1.0f-value;
			_controller->SetSequenceMaster(int(value*255.0f), _src);
		}
		
		virtual float Get() const {
			float v = float(_controller->GetSequenceMaster(_src))/255.0f;
			return _invert?(1.0f-v):v;
		}

		virtual std::wstring GetAddress() const { 
			return (_invert?L"-":L"")+std::wstring(L"sm");
		}

		virtual float GetResult() const {
			float v = _controller->GetSequenceMasterValue();
			return _invert?(1.0f-v):v;
		}

		virtual DMXMacroType GetType() const {
			return DMXMacro::DMXMacroTypeMaster;
		}

		virtual float GetResultCached() const {
			float v = _controller->GetSequenceMasterValue();
			return _invert?(1.0f-v):v;
		}

	protected:
		DMXController* _controller;
		DMXSource _src;
		bool _invert;
};

DMXController::DMXController() {
	_channelCount = 0;
	_values = 0;

	Reset();
}

void DMXController::Reset() {
	ThreadLock lock(&_lock);

	_macro.clear();
	SetChannelCount(512,true);
	_grandMaster._manual = 255;
	_grandMaster._sequence = 0;
	_sequenceMaster._manual = 255;
	_sequenceMaster._sequence = 0;

	_allDirty = false;
	_highestChannelUsed = 1;
	_modificationID = 0;
	_switchingSlots.clear();
}

unsigned int DMXController::GetUniverseCount() const {
	return _channelCount/512;
}

void DMXController::Load(TiXmlElement* you) {
	ThreadLock lock(&_lock);
	unsigned int n = LoadAttributeSmall<unsigned int>(you, "universes", GetUniverseCount());
	SetUniverseCount(n);

	// Load which channels should be switching
	TiXmlElement* switchingElement = you->FirstChildElement("switching");
	if(switchingElement!=0) {
		TiXmlElement* channel = switchingElement->FirstChildElement("channel");
		while(channel!=0) {
			int ds = LoadAttributeSmall(channel, "id", -1);
			if(ds>=0) {
				_switchingSlots.insert((DMXSlot)ds);
			}
			channel = channel->NextSiblingElement("channel");
		}
	}

	std::map< std::wstring, ref<DMXDevice> > devs;
	std::set< ref<DMXDeviceClass> >::iterator it = _classes.begin();

	while(it!=_classes.end()) {
		ref<DMXDeviceClass> dc = *it;
		ref< std::vector< ref<DMXDevice> > > availableDevices = dc->GetAvailableDevices();
		if(availableDevices) {
			std::vector< ref<DMXDevice> >::iterator ait = availableDevices->begin();
			while(ait!=availableDevices->end()) {
				ref<DMXDevice> device = *ait;
				if(device) {
					devs[device->GetDeviceID()] = device;
				}
				++ait;
			}
		}
		++it;
	}

	TiXmlElement* device = you->FirstChildElement("device");
	while(device!=0) {
		std::wstring id = LoadAttributeSmall<std::wstring>(device, "id", L"");
		bool enabled = LoadAttributeSmall<bool>(device, "enabled", false);
		
		std::map< std::wstring, ref<DMXDevice> >::iterator dit = devs.find(id);
		if(dit!=devs.end()) {
			ref<DMXDevice> ddevice = dit->second;
			if(ddevice) {
				ddevice->Load(device);
				if(enabled) {
					AddDevice(ddevice);
				}
			}
		}

		device = device->NextSiblingElement("device");
	}
}

void DMXController::Save(TiXmlElement* you) {
	ThreadLock lock(&_lock);
	SaveAttributeSmall<unsigned int>(you, "universes", GetUniverseCount());

	// Store which channels should be 'switching'
	if(_switchingSlots.size()>0) {
		TiXmlElement switchingElement("switching");
		std::set<DMXSlot>::const_iterator it = _switchingSlots.begin();
		while(it!=_switchingSlots.end()) {
			DMXSlot slot = *it;
			TiXmlElement channelElement("channel");
			SaveAttributeSmall(&channelElement, "id", slot);
			switchingElement.InsertEndChild(channelElement);
			++it;
		}
		you->InsertEndChild(switchingElement);
	}

	std::set< ref<DMXDeviceClass> >::iterator it = _classes.begin();
	while(it!=_classes.end()) {
		ref<DMXDeviceClass> dc = *it;
		ref< std::vector< ref<DMXDevice> > > availableDevices = dc->GetAvailableDevices();
		if(availableDevices) {
			std::vector< ref<DMXDevice> >::iterator ait = availableDevices->begin();
			while(ait!=availableDevices->end()) {
				ref<DMXDevice> device = *ait;
				TiXmlElement eDevice("device");
				SaveAttributeSmall<std::wstring>(&eDevice, "id", device->GetDeviceID());
				SaveAttributeSmall<bool>(&eDevice, "enabled", IsDeviceEnabled(device));
				device->Save(&eDevice);
				you->InsertEndChild(eDevice);
				++ait;
			}
		}
		++it;
	}
}

void DMXController::SetUniverseCount(unsigned int n) {
	if(n<1) return;
	SetChannelCount(n*512);
}

void DMXController::SetChannelCount(unsigned int c, bool reset) {
	ThreadLock lock(&_lock);
	ThreadLock lockTransmit(&_transmitLock);

	if(c==_channelCount && !reset) return; // nothing to do...
	volatile DMXController::DMXChannel* data = new DMXChannel[c];

	if(_channelCount>0) {
		if(reset) {
			for(unsigned int a=0;a<c;a++) {
				data[a]._dirty = true;
				data[a]._manual = 0;
				data[a]._sequence = 0;
			}
		}
		else {
			// copy over the old values
			for(unsigned int a=0;(a<_channelCount && a<c);a++) {
				data[a]._dirty = _values[a]._dirty;
				data[a]._manual = _values[a]._manual;
				data[a]._sequence = _values[a]._sequence;
			}
		}
		delete[] _values;
		delete[] _transmit;
	}
	_values = data;

	// Create new transmit buffer and set dirty flags
	_transmit = new unsigned char[c];
	for(unsigned int a=0;a<c;a++) {
		_transmit[a] = 0;
	}

	_channelCount = c;
	_allDirty = true;
	_anyDirty = true;
}

void DMXController::AddDevice(ref<DMXDevice> d) {
	ThreadLock lock(&_lock);
	_devices.insert(d);
	d->SetController(this);
	d->Start();
}

bool DMXController::IsDeviceEnabled(ref<DMXDevice> d) const {
	return _devices.find(d)!=_devices.end();
}

void DMXController::ToggleDevice(ref<DMXDevice> d) {
	ThreadLock lock(&_lock);
	std::set< ref<DMXDevice> >::iterator it = _devices.find(d);
	if(it!=_devices.end()) {
		_devices.erase(it);
		d->Stop();
	}
	else {
		AddDevice(d);
	}
}

void DMXController::RemoveDevice(ref<DMXDevice> d) {
	ThreadLock lock(&_lock);

	std::set< ref<DMXDevice> >::iterator it = _devices.find(d);
	if(it!=_devices.end()) {
		_devices.erase(it);
	}

	d->Stop();
}

void DMXController::AddDeviceClass(ref<DMXDeviceClass> dc) {
	_classes.insert(dc);
}

void DMXController::RemoveDeviceClass(ref<DMXDeviceClass> dc) {
	std::set< ref<DMXDeviceClass> >::iterator it = _classes.find(dc);
	if(it!=_classes.end()) {
		_classes.erase(it);
	}
}

std::set< ref<DMXDeviceClass> >& DMXController::GetDeviceClasses() {
	return _classes;
}

int DMXController::GetTotalChannelCount() const {
	return _channelCount;
}

bool DMXController::IsSwitching(DMXSlot channel) const {
	return _switchingSlots.find(channel) != _switchingSlots.end();
}

void DMXController::SetSwitching(DMXSlot channel, bool s) {
	ThreadLock lock(&_lock);
	if(s) {
		_switchingSlots.insert(channel);
	}
	else {
		_switchingSlots.erase(channel);
	}
	_allDirty = true; // TODO: replace this with _dirty[channel] = true? need bound-checking on channel then!
}

void DMXController::SetSwitching(const std::set<DMXSlot>& scs) {
	ThreadLock lock(&_lock);
	_switchingSlots = scs;
	_allDirty = true;
}

void DMXController::GetSwitching(std::set<DMXSlot>& scs) const {
	scs = _switchingSlots;
}

ref<DMXMacro> DMXController::CreateMacro(std::wstring address, DMXSource source) {
	ThreadLock lock(&_lock);
	_allDirty = true;
	std::wstring original = address;

	if(address.length()<1) {
		return GC::Hold(new NullDMXMacro());
	}

	ref<DMXMacro> macro;
	std::transform(address.begin(), address.end(), address.begin(), tolower);

	bool invert = false;
	if(address.at(0)==L'-') {
		invert = true;
		address = address.substr(1);
	}

	bool precise = false;
	if(address.at(0)==L'p') {
		precise = true;
		address = address.substr(1);
	}

	bool submix = false;
	if(address.at(0)==L'M'||address.at(0)==L'm') {
		submix = true;
		address = address.substr(1);
	}

	// 'gm' and 'sm' cannot be used in a macro, their channel numbers can
	if(address==std::wstring(L"gm")) {
		macro = GC::Hold(new GrandMasterMacro(this, source, invert));
	}
	else if(address==std::wstring(L"sm")) {
		macro = GC::Hold(new SequenceMasterMacro(this, source, invert));
	}
	else if(address.find(L",")==std::wstring::npos) {
		DMXSlot channel = ParseChannelNumber(address);
		if(channel>0 && channel <= _channelCount) {
			_highestChannelUsed = Util::Max(_highestChannelUsed, channel);
			if(precise) {
				macro = GC::Hold(new PreciseDMXMacro(this, channel, source, invert));
			}
			else {
				macro = GC::Hold(new SimpleDMXMacro(this, channel, source, invert));
		
			}
		}
		else {
			macro = GC::Hold(new NullDMXMacro());
		}
	}
	else {
		macro = GC::Hold(new ComplexDMXMacro(this, original, source, invert));
		
		if(_macro.find(original) == _macro.end()) {
			_macro[original] = MacroInfo(submix?1.0f:0.0f, 0.0f, submix);
		}
	}

	++_modificationID;
	return macro;
} 

DMXSlot DMXController::ParseChannelNumber(const std::wstring& n) {
	std::wstring::size_type nt = n.find(L".");
	if(nt==std::wstring::npos) {
		return StringTo<DMXSlot>(n, -1);
	}
	else {
		std::wstring universe(n, 0, nt);
		std::wstring chan(n, nt+1);

		DMXSlot ds = (StringTo<DMXSlot>(universe,0) * 512) + StringTo<DMXSlot>(chan,0);
		return ds;
	}
}

void DMXController::Set(const std::wstring& macro, float value, DMXSource source) {
	ThreadLock lock(&_lock);

	std::map<std::wstring, MacroInfo>::iterator it = _macro.find(macro);
	if(it == _macro.end()) {
		return; // all macros are registered in the _macro map
	}

	MacroInfo& mi = it->second;
	switch(source) {
		case DMXManual:
			mi._manual = value;
			break;

		case DMXSequence:
			mi._sequence = value;
			break;
	}

	_allDirty = true;
	_anyDirty = true;
	++_modificationID;
	Transmit();
}

float DMXController::GetMacroResult(const std::wstring& macro) {
	std::map<std::wstring, MacroInfo>::iterator it = _macro.find(macro);
	if(it == _macro.end()) {
		return 0.0f; // all macros are registered in the _macro map
	}

	MacroInfo& mi = it->second;
	return Util::Max(mi._manual,mi._sequence);
}

void DMXController::DestroyMacro(DMXMacro* macroptr) {
	ThreadLock lock(&_lock);
	std::wstring addr = macroptr->GetAddress();

	std::map<std::wstring, MacroInfo>::iterator it = _macro.find(addr);
	if(it!=_macro.end()) {
		_macro.erase(it);
	}
}

bool DMXController::IsMacroSubmix(const std::wstring& name) {
	std::map<std::wstring, MacroInfo>::iterator it = _macro.find(name);
	if(it == _macro.end()) {
		return false;
	}
	return it->second._submix;
}

int DMXController::Get(const std::wstring& macro, DMXSource src) {
	std::map<std::wstring, MacroInfo>::iterator it = _macro.find(macro);
	if(it == _macro.end()) {
		return -1; // all macros are registered in the _macro map
	}

	MacroInfo& mi = it->second;

	switch(src) {
		case DMXManual:
			return int(mi._manual*255.0f);

		case DMXSequence:
			return int(mi._sequence*255.0f);
	}

	return 0;
}

inline int DMXController::GetChannelResult(DMXSlot ch) {
	std::wostringstream chss;
	chss << L"," << int(ch) << L",";
	std::wstring chs = chss.str();

	float a = float(Get(ch, DMXManual)) / 255.0f;
	float b = float(Get(ch, DMXSequence)) / 255.0f;

	// multiply sequence value with sequence master
	b *= GetSequenceMasterValue();

	float highest = Util::Max(a,b);
		
	// process submix macro's
	float submixRatio = 1.0f;

	if(ch > 0 && ch <= _channelCount) {
		submixRatio *= GetGrandMasterValue();
	}

	if(_macro.size()>0) {
		std::map<std::wstring, MacroInfo>::iterator it = _macro.begin();
		while(it!=_macro.end()) {
			std::wstring address = it->first;
			std::wstring originalAddress = address;
			if(address.at(0)==L'M'||address.at(0)==L'm') {
				address = address.substr(1);
			}

			MacroInfo& mi = it->second;
			address = L","+address+L",";

			// is our channel in the address?
			if(address.find(chs)!=std::wstring::npos) {
				float val = Util::Max(mi._manual, mi._sequence);
				if(mi._submix) {
					submixRatio *= val;
				}
				else {
					highest = Util::Max(val, highest);
				}
			}

			++it;
		}
	}

	if(IsSwitching(ch)) {
		return (int(submixRatio*highest*255.0f) >= 127) ? 255 : 0;
	}
	else {
		return int(submixRatio*highest*255.0f);
	}
}

float DMXController::GetGrandMasterValue() const {
	return float(Util::Max(_grandMaster._manual, _grandMaster._sequence))/255.0f;
}

float DMXController::GetSequenceMasterValue() const {
	return float(Util::Max(_sequenceMaster._manual, _sequenceMaster._sequence))/255.0f;
}

int DMXController::GetChannelResultCached(DMXSlot ch) {
	if(ch<=0 || ch > int(_channelCount)) {
		return -1;
	}

	return (int)_transmit[ch-1];
}

void DMXController::Process() {
	ThreadLock lock(&_transmitLock);
	if(_anyDirty) {
		float master = GetGrandMasterValue();

		if(master<=0.0f) {
			for(unsigned int a=0;a<_channelCount;a++) {
				_transmit[a] = 0;
			}
		}
		else {
			for(DMXSlot a=1;a<_highestChannelUsed+1;a++) {
				volatile DMXChannel& channel = _values[a-1];
				if(channel._dirty || _allDirty) {
					_transmit[a-1] = (unsigned char)(int(float(GetChannelResult(a)) * master));
					channel._dirty = false;
				}
			}
		}

		_allDirty = false;
		_anyDirty = false;
	}
}

void DMXController::Transmit() {
	ThreadLock lock(&_transmitLock);
	std::set< ref<DMXDevice> >::iterator it = _devices.begin();
	while(it!=_devices.end()) {
		ref<DMXDevice> device = *it;
		device->Transmit();
		++it;
	}
}

int DMXController::Get(DMXSlot channel, DMXSource src) {
	if(channel<=0 || channel > int(_channelCount)) {
		return 0;
	}

	const volatile DMXChannel& value = _values[channel-1];

	if(src==DMXManual) {
		return value._manual;
	}
	else {
		return Util::Max(value._manual, value._sequence);
	}
}

int DMXController::GetGrandMaster(DMXSource src) {
	if(src==DMXManual) {
		return _grandMaster._manual;
	}
	else {
		return Util::Max(_grandMaster._manual, _grandMaster._sequence);
	}
}

int DMXController::GetSequenceMaster(DMXSource src) {
	if(src==DMXManual) {
		return _sequenceMaster._manual;
	}
	else {
		return Util::Max(_sequenceMaster._manual, _sequenceMaster._sequence);
	}
}

void DMXController::SetGrandMaster(int value, DMXSource src) {
	if(value<0 || value>255) return;

	switch(src) {
		case DMXManual:
			_grandMaster._manual = value;
			break;
		
		case DMXSequence:
			_grandMaster._sequence = value;
			break;
	}

	_grandMaster._dirty = true;
	_anyDirty = true;
	_allDirty = true;
	++_modificationID;
	Transmit();
}

void DMXController::SetSequenceMaster(int value, DMXSource src) {
	if(value<0 || value>255) return;

	switch(src) {
		case DMXManual:
			_sequenceMaster._manual = value;
			break;
		
		case DMXSequence:
			_sequenceMaster._sequence = value;
			break;
	}

	_sequenceMaster._dirty = true;
	_anyDirty = true;
	_allDirty = true;
	++_modificationID;
	Transmit();
}

// Channel 0 is Grand Master
void DMXController::Set(DMXSlot channel, int value, DMXSource src) {
	if(channel<=0 || channel > int(_channelCount) || value <0 || value > 255) {
		return;
	}

	if(src==DMXManual) {
		ThreadLock lock(&_lock);
		_values[channel-1]._manual = (unsigned char)value;
	}
	else {
		ThreadLock lock(&_lock);
		_values[channel-1]._sequence = (unsigned char)value;
	}

	_values[channel-1]._dirty = true;
	_anyDirty = true;
	++_modificationID;
	Transmit();
}

DMXController::~DMXController()  {
}

unsigned int DMXController::GetModificationID() const {
	return _modificationID;
}

unsigned char const* DMXController::GetTransmitBuffer() const {
	return _transmit;
}