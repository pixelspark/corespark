#include "../include/tjslip.h"
using namespace tj::shared;
using namespace tj::np;

const unsigned char SLIPFrameDecoder::KSLIPEndCharacter = 0xC0;
const unsigned char SLIPFrameDecoder::KSLIPEscapeCharacter = 0xDB;
const unsigned char SLIPFrameDecoder::KSLIPEscapeEscapeCharacter = 0xDD;
const unsigned char SLIPFrameDecoder::KSLIPEscapeEndCharacter = 0xDC;

SLIPFrameDecoder::SLIPFrameDecoder(): _isReceivingPacket(false), _lastCharacterWasEscape(false) {
}

SLIPFrameDecoder::~SLIPFrameDecoder() {
}

void SLIPFrameDecoder::Append(const unsigned char* data, unsigned int length) {
	if(!_buffer) {
		_buffer = GC::Hold(new CodeWriter());
	}

	unsigned int index = 0;
	while(index < length) {
		if(_lastCharacterWasEscape && _isReceivingPacket) {
			if(data[index]==KSLIPEscapeEscapeCharacter) {
				_buffer->Add((unsigned char)KSLIPEscapeCharacter);
			}
			else if(data[index]==KSLIPEscapeEndCharacter) {
				_buffer->Add((unsigned char)KSLIPEndCharacter);
			}
			else {
				Log::Write(L"TJNP/SLIPFrameDecoder", L"Invalid code after escape character, dropping packet");
				_isReceivingPacket = false;
				_buffer->Reset();
			}
			_lastCharacterWasEscape = false;
		}
		else if(data[index]==KSLIPEndCharacter) {
			if(_isReceivingPacket) {
				// A message was being received and has now ended
				if(_buffer->GetSize()>0) {
					OnPacketReceived((const unsigned char*)_buffer->GetBuffer(), _buffer->GetSize());
				}
				_isReceivingPacket = false;
			}
			else {
				_isReceivingPacket = true;
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
			// Read characters until we encounter either the ESC or END character
			unsigned int endIndex = index+1;
			while(endIndex < length) {
				if(data[endIndex]==KSLIPEscapeCharacter || data[endIndex]==KSLIPEndCharacter) {
					break;
				}
				endIndex++;
			}
			unsigned int dataLength = endIndex-1-index;
			if(dataLength>0) {
				_buffer->Append((const char*)&(data[index]), dataLength);
			}
			index = endIndex;
		}
		else {
			Log::Write(L"TJNP/SLIPFrameDecoder", L"Spurious characters outside packet, ignoring");
		}
		++index;
	}
}

void SLIPFrameDecoder::OnPacketReceived(const unsigned char* data, unsigned int length) {
}