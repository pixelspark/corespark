#include "../include/tjnp.h"
using namespace tj::np;
using namespace tj::shared;

Message::Message(bool toPlugin) {
	_header = 0;
	_writer = CreateWriter();
	_sent = false;
	_toPlugin = toPlugin;
}

Message::~Message() {
}

bool Message::IsSent() {
	return _sent;
}

void Message::SetSent() {
	_sent = true;
}

bool Message::IsSentToPlugin() const {
	return _toPlugin;
}


ref<CodeWriter> Message::CreateWriter() {
	ref<CodeWriter> cw = GC::Hold(new CodeWriter());
	PacketHeader ph;
	ph._action = ActionUpdate;
	cw->Add(ph);
	_header = (PacketHeader*)cw->_buffer;
	return cw;
}

PacketHeader* Message::GetHeader() {
	return _header;
}

void Message::SetChannel(Channel id) {
	_header->_channel = id;
}

void Message::SetPlugin(PluginHash id) {
	_header->_plugin = id;
}

unsigned int Message::GetSize() {
	return _writer->GetSize();
}

const char* Message::GetBuffer() {
	return _writer->GetBuffer();
}