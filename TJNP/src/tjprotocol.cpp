#include "../include/tjnp.h"
#include <memory>

using namespace tj::np;
using namespace tj::shared;

/* Packet */
Packet::Packet(const PacketHeader& ph, const char* message, unsigned int size) {
	_size = size;
	_message = new char[size];
	memcpy(_message, message, size*sizeof(char));
	_header = ph;
}

Packet::~Packet() {
	delete[] _message;
	_size = 0;
}

unsigned int Packet::GetSize() {
	return _size;
}

PacketHeader Packet::GetHeader() {
	return _header;
}

const char* Packet::GetMessage() {
	return _message;
}

PacketHeader::PacketHeader() {
	_action = ActionNothing;
	_channel = 0;
	_size = 0;
	_version[0] = 'T';
	_version[1] = 'N';
	_version[2] = 'P';
	_version[3] = '3';
	_transaction = 0;
	_from = 0;
	_plugin = 0;
}