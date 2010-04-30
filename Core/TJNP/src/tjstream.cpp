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
 
 #include "../include/tjstream.h"
using namespace tj::np;
using namespace tj::shared;

Message::Message(PacketAction ac, TransactionIdentifier ti): _writer(GC::Hold(new DataWriter())), _sent(false) {
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

Message::Message(bool toPlugin, const GroupID& gid, const Channel& cid, const PluginHash& plh): _writer(GC::Hold(new DataWriter())), _sent(false) {
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

strong<Packet> Message::ConvertToPacket() {
	Bytes dataSize = _writer->GetSize();
	char* data = _writer->TakeOverBuffer(true);
	strong<Packet> packet = GC::Hold(new Packet(data, (unsigned int)dataSize));
	_header = 0;
	return packet;
}

bool Message::IsSent() const {
	return _sent;
}

void Message::SetSent() {
	_sent = true;
}

PacketHeader* Message::GetHeader() {
	if(_header==0) {
		Throw(L"Message was converted to Packet, cannot change header anymore", ExceptionTypeSevere);
	}
	return _header;
}

unsigned int Message::GetSize() {
	if(_header==0) {
		Throw(L"Message was converted to Packet, cannot get size anymore", ExceptionTypeSevere);
	}
	return (unsigned int)_writer->GetSize();
}

const char* Message::GetBuffer() {
	if(_header==0) {
		Throw(L"Message was converted to Packet, cannot change contents anymore", ExceptionTypeSevere);
	}
	return _writer->GetBuffer();
}