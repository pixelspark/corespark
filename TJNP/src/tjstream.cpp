#include "../include/tjnp.h"
using namespace tj::np;
using namespace tj::shared;

Message::Message(PacketAction ac, TransactionIdentifier ti): _writer(GC::Hold(new CodeWriter())), _sent(false) {
	// Construct packet header
	PacketHeader ph;
	ph._action = ac;
	ph._channel = 0;
	ph._group = 0;
	ph._plugin = 0;
	ph._transaction = ti;
	_writer->Add(ph);
	_header = (PacketHeader*)_writer->_buffer;
}

Message::Message(bool toPlugin, const GroupID& gid, const Channel& cid, const PluginHash& plh): _writer(GC::Hold(new CodeWriter())), _sent(false) {
	// Construct packet header
	PacketHeader ph;
	ph._action = toPlugin ? ActionUpdatePlugin : ActionUpdate;
	ph._channel = cid;
	ph._group = gid;
	ph._plugin = plh;
	_writer->Add(ph);
	_header = (PacketHeader*)_writer->_buffer;
}

Message::~Message() {
}

bool Message::IsSent() const {
	return _sent;
}

void Message::SetSent() {
	_sent = true;
}

PacketHeader* Message::GetHeader() {
	return _header;
}

unsigned int Message::GetSize() {
	return _writer->GetSize();
}

const char* Message::GetBuffer() {
	return _writer->GetBuffer();
}