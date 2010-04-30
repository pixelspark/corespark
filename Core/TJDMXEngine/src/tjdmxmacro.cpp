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
 
 #include "../include/tjdmxmacro.h"
using namespace tj::dmx;

DMXMacro::~DMXMacro() {
}

float DMXMacro::GetResultCached() const {
	return GetResult();
}

/* SimpleDMXMacro */
SimpleDMXMacro::SimpleDMXMacro(DMXController* controller, int channel, DMXSource src, bool invert) {
	assert(_controller!=0);
	_controller = controller;
	_channel = channel;
	_source = src;
	_invert = invert;
	_address = Stringify(channel);
}

SimpleDMXMacro::~SimpleDMXMacro() {
}

void SimpleDMXMacro::Set(float value) {
	if(_invert) value = 1.0f-value;
	_controller->Set(_channel, int(value*255.0f), _source);
}

float SimpleDMXMacro::Get() const {
	int v = _controller->Get(_channel, _source);
	if(_invert) v = 255-v;
	return float(v)/255.0f;
}

std::wstring SimpleDMXMacro::GetAddress() const {
	return _address;
}

float SimpleDMXMacro::GetResult() const {
	int v = _controller->GetChannelResult(_channel);
	if(_invert) v = 255-v;
	return float(v)/255.0f;
}

float SimpleDMXMacro::GetResultCached() const {
	int v = _controller->GetChannelResultCached(_channel);
	if(_invert) v = 255-v;
	return float(v)/255.0f;
}

DMXMacro::DMXMacroType SimpleDMXMacro::GetType() const {
	if(_channel==0 || _channel==513) {
		return DMXMacroTypeMaster;
	}
	return DMXMacroTypeNormal;
}

/* PreciseDMXMacro */
PreciseDMXMacro::PreciseDMXMacro(DMXController* c, int channel, DMXSource src, bool invert) {
	_controller = c;
	_invert = invert;
	_channel = channel;
	_source = src;
}

PreciseDMXMacro::~PreciseDMXMacro() {
}

void PreciseDMXMacro::Set(float f) {
	if(_invert) f = 1.0f-f;
	Set(int(f*65535));
}

void PreciseDMXMacro::Set(int v) {
	if(v<0 || v>65535) return;

	int coarse = v / 256;
	int fine = v - (coarse*256);

	_controller->Set(_channel, coarse, _source);
	_controller->Set(_channel+1, fine, _source);
}

float PreciseDMXMacro::Get() const {
	int coarse = _controller->Get(_channel, _source);
	int fine = _controller->Get(_channel+1, _source);
	float v = float(coarse*256 + fine)/65535.0f;
	return _invert?(1.0f-v):v;
}

float PreciseDMXMacro::GetResult() const {
	float v = float(_controller->GetChannelResult(_channel)*255 + _controller->GetChannelResult(_channel+1))/65536.0f;
	return _invert?(1.0f-v):v;
}

std::wstring PreciseDMXMacro::GetAddress() const {
	return L"p" + Stringify(_channel);
}

DMXMacro::DMXMacroType PreciseDMXMacro::GetType() const {
	return DMXMacroTypeNormal;
}

/* ComplexDMXMacro */
ComplexDMXMacro::ComplexDMXMacro(DMXController* controller, std::wstring address, DMXSource src, bool invert) {
	_controller = controller;
	_address = address;
	_source = src;
	_invert = invert;

	if(address.length()<1) return;

	if(address.at(0)==L'-') {
		address = address.substr(1);
	}

	if(address.at(0)==L'm') {
		address = address.substr(1);
	}
	// parse address
	std::vector<std::wstring> parts = Explode<std::wstring>(address, std::wstring(L","));
	std::vector<std::wstring>::iterator it = parts.begin();
	while(it!=parts.end()) {
		std::wstring nr = *it;
		DMXSlot channel = DMXController::ParseChannelNumber(nr);
		
		if(channel>-1 && channel < 514) {
			_controller->_highestChannelUsed = Util::Max(_controller->_highestChannelUsed, channel);
			_channels.push_back(channel);
		}

		++it;
	}
}

ComplexDMXMacro::~ComplexDMXMacro() {
	// if a manual macro is destroyed, it usually means that the macro is not needed anymore
	if(_source==DMXManual) {
		_controller->DestroyMacro(this);
	}
}

void ComplexDMXMacro::Set(float value) {
	if(value < 0.0f || value > 1.0f) {
		Log::Write(L"TJDMXController/ComplexMacro", std::wstring(L"Set float used with invalid value:")+Stringify(value));
	}

	if(_invert) value = 1.0f-value;
	_controller->Set(_address, value, _source);
}

float ComplexDMXMacro::Get() const {
	int v = _controller->Get(_address, _source);
	if(_invert) v = 255-v;
	return float(v)/255.0f;
}

DMXMacro::DMXMacroType ComplexDMXMacro::GetType() const {
	return _controller->IsMacroSubmix(_address)? DMXMacroTypeSubmix : DMXMacroTypeScene;
}

std::wstring ComplexDMXMacro::GetAddress() const {
	return _address;
}

float ComplexDMXMacro::GetResult() const {
	int v = int(_controller->GetMacroResult(_address)*255.0f);
	if(_invert) v = 255 - v;
	return float(v)/255.0f;
}


/* NullDMXMacro */
NullDMXMacro::NullDMXMacro() {
	_value = 0.0f;
}

NullDMXMacro::~NullDMXMacro() {
}

void NullDMXMacro::Set(float value) {
	_value = value;
}

float NullDMXMacro::Get() const {
	return _value;
}

DMXMacro::DMXMacroType NullDMXMacro::GetType() const {
	return DMXMacroTypeNormal;
}

std::wstring NullDMXMacro::GetAddress() const {
	return L"";
}

float NullDMXMacro::GetResult() const {
	return _value;
}