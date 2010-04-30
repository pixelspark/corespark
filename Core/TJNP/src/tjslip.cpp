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
 
 #include "../include/tjslip.h"
using namespace tj::shared;
using namespace tj::np;

const unsigned char SLIPFrameDecoder::KSLIPEndCharacter = 0xC0;
const unsigned char SLIPFrameDecoder::KSLIPEscapeCharacter = 0xDB;
const unsigned char SLIPFrameDecoder::KSLIPEscapeEscapeCharacter = 0xDD;
const unsigned char SLIPFrameDecoder::KSLIPEscapeEndCharacter = 0xDC;

QueueSLIPFrameDecoder::QueueSLIPFrameDecoder() {
}

QueueSLIPFrameDecoder::~QueueSLIPFrameDecoder() {
}

ref<DataReader> QueueSLIPFrameDecoder::NextPacket() {
	std::deque< ref<DataReader> >::iterator eit = _buffers.begin();
	if(eit!=_buffers.end()) {
		ref<DataReader> code = *eit;
		_buffers.erase(eit);
		return code;
	}
	return null;
}

void QueueSLIPFrameDecoder::OnPacketReceived(const unsigned char* data, Bytes len) {
	_buffers.push_back(GC::Hold(new DataReader((const char*)data, len)));
}

SLIPFrameDecoder::SLIPFrameDecoder(): _isReceivingPacket(false), _lastCharacterWasEscape(false), _isDiscardingPacket(false) {
}

SLIPFrameDecoder::~SLIPFrameDecoder() {
}

void SLIPFrameDecoder::EncodeSLIPFrame(const unsigned char* data, Bytes length, strong<DataWriter> cw) {
	cw->Add(KSLIPEndCharacter);
	Bytes index = 0;
	while(index<length) {
		if(data[index]==KSLIPEndCharacter) {
			cw->Add(KSLIPEscapeCharacter);
			cw->Add(KSLIPEscapeEndCharacter);
		}
		else if(data[index]==KSLIPEscapeCharacter) {
			cw->Add(KSLIPEscapeCharacter);
			cw->Add(KSLIPEscapeEscapeCharacter);
		}
		else {
			Bytes endIndex = index+1;
			while(endIndex < length) {
				if(data[endIndex]==KSLIPEscapeCharacter || data[endIndex]==KSLIPEndCharacter) {
					break;
				}
				endIndex++;
			}
			Bytes dataLength = endIndex-index;
			if(dataLength>0) {
				cw->Append((const char*)&(data[index]), dataLength);
			}
			index = endIndex-1;
		}
		++index;
	}
	cw->Add(KSLIPEndCharacter);
}

void SLIPFrameDecoder::Append(const unsigned char* data, Bytes length) {
	if(!_buffer) {
		_buffer = GC::Hold(new DataWriter());
	}

	Bytes index = 0;
	while(index < length) {
		if(_lastCharacterWasEscape && _isReceivingPacket) {
			if(data[index]==KSLIPEscapeEscapeCharacter) {
				_buffer->Add((unsigned char)KSLIPEscapeCharacter);
			}
			else if(data[index]==KSLIPEscapeEndCharacter) {
				_buffer->Add((unsigned char)KSLIPEndCharacter);
			}
			else {
				Log::Write(L"TJNP/SLIPFrameDecoder", L"Invalid code after escape character (idx="+Stringify(index)+L"), dropping packet");
				_isDiscardingPacket = true;
				_buffer->Reset();
			}
			_lastCharacterWasEscape = false;
		}
		else if(data[index]==KSLIPEndCharacter) {
			if(_isReceivingPacket) {
				// A message was being received and has now ended
				if(!_isDiscardingPacket && _buffer->GetSize()>0) {
					// End of a (valid) packet
					OnPacketReceived((const unsigned char*)_buffer->GetBuffer(), _buffer->GetSize());
				}
				_isReceivingPacket = false;
				_isDiscardingPacket = false;
			}
			else {
				// Start of new packet
				_isReceivingPacket = true;
				_isDiscardingPacket = false;
			}

			_buffer->Reset();
			_lastCharacterWasEscape = false;
		}
		else if(data[index]==KSLIPEscapeCharacter) {
			if(_isReceivingPacket) {
				_lastCharacterWasEscape = true;
			}
			else {
				// Escape character makes no sense outside a packet, of course
				Log::Write(L"TJNP/SLIPFrameDecoder", L"Escape character outside packet, ignoring");
			}
		}
		else if(_isReceivingPacket) {
			if(!_isDiscardingPacket) {
				// Read characters until we encounter either the ESC or END character
				Bytes endIndex = index+1;
				while(endIndex < length) {
					if(data[endIndex]==KSLIPEscapeCharacter || data[endIndex]==KSLIPEndCharacter) {
						break;
					}
					endIndex++;
				}

				Bytes dataLength = endIndex-index;
				if(dataLength>0) {
					_buffer->Append((const char*)&(data[index]), dataLength);
				}
				index = endIndex-1;
			}
		}
		else {
			Log::Write(L"TJNP/SLIPFrameDecoder", L"Spurious characters outside packet, ignoring");
		}
		++index;
	}
}

void SLIPFrameDecoder::OnPacketReceived(const unsigned char* data, Bytes length) {
}