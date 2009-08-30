#include "../include/tjnp.h"

#include <time.h>
#include <sstream>

#ifndef TJ_OS_WIN
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define _strdup strdup
#else
	typedef int socklen_t;
#endif

using namespace tj::shared;
using namespace tj::np;

#pragma pack(push,1)

#ifdef TJ_OS_WIN
	LRESULT CALLBACK SocketMessageWindowProc(HWND, UINT, WPARAM, LPARAM);
	#define TJSOCKET_MESSAGE_CLASS (L"TjSocketMessageWnd")
	#define TJSOCKET_MESSAGE (WM_USER+1338)
#endif

NetworkInitializer Socket::_initializer;

Socket::Socket(int port, const char* address, ref<Node> nw): _lastPacketID(0), _bytesSent(0), _bytesReceived(0), _network(nw), _maxReliablePacketCount(KDefaultMaxReliablePacketCount) {
	// Create a random transaction counter id
	_transactionCounter = rand();
	_initializer.Initialize();
	assert(address!=0 && port > 0 && port < 65536);
	_recieveBuffer = new char[Packet::maximumSize];

	_port = port;
	_bcastAddress = _strdup(address);

	sockaddr_in addr;
	_client = 0;
	_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(_server == INVALID_SOCKET) {
		Throw(L"Couldn't create a server socket", ExceptionTypeError);
	}

	// Fill in the interface information
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)port);
	addr.sin_addr.s_addr = INADDR_ANY;

	int on = 1;
	setsockopt(_server,SOL_SOCKET,SO_REUSEADDR,(const char*)&on, sizeof(int));

	if(bind(_server,(sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		Log::Write(L"TJShow/Network", Stringify(_bcastAddress) + L" could not connect");
		Throw(L"Couldn't create socket for listening. ", ExceptionTypeError);
		return;
	}

	// make us member of the multicast group for TJShow
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr((const char*)_bcastAddress);
	mreq.imr_interface.s_addr = INADDR_ANY;
	setsockopt(_server, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));

	#ifdef TJ_OS_WIN
		_window = CreateWindow(TJSOCKET_MESSAGE_CLASS, L"SocketWnd", 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0);
		if(!_window) {
			Throw(L"Couldn't create message window for socket.", ExceptionTypeError);
		}
		SetWindowLong(_window, GWL_USERDATA, LONG((long long)this));
		WSAAsyncSelect(_server,_window,TJSOCKET_MESSAGE,FD_READ);
	#else
		#error Not implemented (socket select operation)
	#endif

	_client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(_client == INVALID_SOCKET) {
		Throw(L"Couldn't open socket for broadcasting",ExceptionTypeError);
		return;
	}

	setsockopt(_client,SOL_SOCKET,SO_BROADCAST,(const char*)&on, sizeof(int));
	setsockopt(_client,SOL_SOCKET,SO_REUSEADDR,(const char*)&on, sizeof(int));
}

Socket::~Socket() {
	#ifdef TJ_OS_WIN
		closesocket(_client);
		closesocket(_server);
		DestroyWindow(_window);
	#else
		close(_client);
		close(_server);
	#endif
	
	delete[] _recieveBuffer;
	delete _bcastAddress;
}

int Socket::GetPort() const {
	return _port; 
}

std::wstring Socket::GetAddress() const {
	return Wcs(std::string(_bcastAddress));
}

void Socket::CleanTransactions() {
	ThreadLock lock(&_lock);

	std::map<TransactionIdentifier, ref<Transaction> >::iterator it = _transactions.begin();
	while(it!=_transactions.end()) {
		ref<Transaction> tx = it->second;
		if(!tx) {
			it = _transactions.erase(it);
		}
		else if(tx->IsExpired()) {
			tx->OnExpire();
			it = _transactions.erase(it);
		}
		else {
			++it;
		}
	}
}

unsigned int Socket::GetActiveTransactionCount() const {
	return (unsigned int)_transactions.size();
}

void Socket::Receive() {
	ref<Node> nw = _network;
	if(!nw) {
		Log::Write(L"TJNP/Socket", L"Internal error: no network instance set");
		return;
	}

	ref<Code> code = 0;
	ref<Transaction> tx = 0;
	sockaddr_in from;
	PacketHeader ph;

	// Receive stuff
	{
		ThreadLock lock(&_lock);
		memset(_recieveBuffer,0,sizeof(char)*Packet::maximumSize);
		socklen_t size = (int)sizeof(from);
		int ret = recvfrom(_server, _recieveBuffer, Packet::maximumSize-1, 0, (sockaddr*)&from, &size);
		
		#ifdef TJ_OS_WIN
			WSAAsyncSelect(_server,_window,TJSOCKET_MESSAGE,FD_READ);
		#else
			#warning Not implemented (part of socket select stuff)
		#endif
		
		if(ret == SOCKET_ERROR) {
			// This seems to happen on packets that come from us
			return;
		}
		_bytesReceived += ret;

		// Extract packet header
		ph = *((PacketHeader*)_recieveBuffer);
		
		// Check if this actually is a T4 packet
		if(ph._version[0]!='T' || ph._version[1] != '4') {
			if(ph._version[0]=='T') {
				Log::Write(L"TJNP/Socket", L"Received a packet which has a different protocol version; cannot process this packet");
			}
			else {
				Log::Write(L"TJNP/Socket", L"Received invalid packets from the network; maybe network link is broken or other applications are running on this port");
			}
			return;
		}

		// Check size
		if(int(ph._size+sizeof(PacketHeader)) > ret) {
			Log::Write(L"TJNP/Socket", L"Packet smaller than it says it is; ignoring it!");
			return;
		}
		
		// Reject loopback messages
		if(ph._from == nw->GetInstanceID()) {
			return;
		}
	
		// If this is a 'special packet', process the flags
		if(ph._flags!=0) {
			// Check if this packet is a re-delivery or a cannot-re-delivery
			if((ph._flags & (PacketFlagCannotRedeliver|PacketFlagRedelivery))!=0) {
				if(ph._flags & PacketFlagCannotRedeliver) {
					///Log::Write(L"TJNP/Socket", L"Packet with rpid="+Stringify(ph._rpid)+L" from "+StringifyHex(ph._from)+L" could not be redelivered; removing from wishlist");
				}
				else {
					///Log::Write(L"TJNP/Socket", L"Packet with rpid="+Stringify(ph._rpid)+L" from "+StringifyHex(ph._from)+L" was redelivered; removing from wishlist");
				}
				std::deque< std::pair<InstanceID, ReliablePacketID> >::iterator it = _reliableWishList.begin();
				bool weRequestedRedelivery = false;
				while(it!=_reliableWishList.end()) {
					if(it->first == ph._from && it->second == ph._rpid) {
						it = _reliableWishList.erase(it);
						weRequestedRedelivery = true;
					}
					else {
						++it;
					}
				}

				if(!weRequestedRedelivery) {
					return; // Packet was redelivered for a different instance on this pc
				}
			}
			else if((ph._flags & PacketFlagRequestRedelivery)!=0) {
				// Redeliver packet with ph._rpid to the sending node and return
				std::map<ReliablePacketID, ref<Packet> >::iterator it = _reliableSentPackets.find(ph._rpid);
				if(it==_reliableSentPackets.end()) {
					///Log::Write(L"TJNP/Socket", L"Sending cannot redeliver (rpid requested="+Stringify(ph._rpid)+L")");
					PacketHeader rph;
					rph._flags = PacketFlagCannotRedeliver;
					rph._from = nw->GetInstanceID();
					rph._transaction = ph._transaction;
					rph._rpid = ph._rpid;
					ref<Packet> response = GC::Hold(new Packet(ph, (const char*)0, 0));
					Send(response, &from, false);
					// Send 'cannot redeliver'
				}
				else {
					if(!(it->second)) {
						///Log::Write(L"TJNP/Socket", L"Cannot redeliver, packet is null!");
					}

					if(ph._plugin==nw->GetInstanceID()) {
						///Log::Write(L"TJNP/Socket", L"Redelivering (rpid requested="+Stringify(ph._rpid)+L")");
						// Send packet with extra 'redelivery' flag, and *only* to the requesting computer
						ref<Packet> packet = it->second;
						packet->_header->_flags = PacketFlagRedelivery;
						packet->_header->_from = nw->GetInstanceID();
						Send(packet, &from, false);
					}
				}
				return;
			}
		}

		// Check reliability properties
		if((ph._flags & PacketFlagReliable) != 0) {
			std::map<InstanceID, ReliablePacketID>::iterator it = _reliableLastReceived.find(ph._from);
			if(it!=_reliableLastReceived.end()) {
				ReliablePacketID predictedNext = ++(it->second);

				if(predictedNext!=ph._rpid) {
					/** TODO: if the packet indicates that it may not be delivered out-of-order, then also put ph._rpid on the wishlist **/
					///Log::Write(L"TJNP/Socket", L"Received packet is out of order (predicted rpid="+Stringify(predictedNext)+L", received rpid="+Stringify(ph._rpid));
					unsigned int n = 0;
					for(ReliablePacketID r = predictedNext; r < ph._rpid; ++r) {
						++n;
						if(_reliableWishList.size() > _maxReliablePacketCount) {
							///Log::Write(L"TJNP/Socket", L"Wish list length exceeded; packets are being dropped!");
							_reliableWishList.pop_front();
						}
						else if(n > _maxReliablePacketCount) {
							///Log::Write(L"TJNP/Socket", L"Wish list length exceeded for this batch; packets are being dropped!");
							break;
						}
						///Log::Write(L"TJNP/Socket", L"Adding reliable packet "+Stringify(r)+L" from "+StringifyHex(ph._from)+L" to receive wish list");
						_reliableWishList.push_back(std::pair<InstanceID, ReliablePacketID>(ph._from, r));
					}
				}
			}
			_reliableLastReceived[ph._from] = ph._rpid;
		}

		// Extract packet contents
		code = GC::Hold(new Code(_recieveBuffer+sizeof(PacketHeader), ph._size));

		// Find our transaction, if it exists. Otherwise, use the 'default transaction' (which happens to be _network)
		if(ph._transaction==0) {
			tx = nw;
		}
		else {
			if(_transactions.find(ph._transaction)!=_transactions.end()) {
				tx = _transactions[ph._transaction];
			}
		}
	}

	// Handle the message
	if(tx && !tx->IsExpired()) {
		tx->OnReceive(ph._from, from.sin_addr, ph, code);
	}
}

void Socket::SendRedeliveryRequests() {
	ThreadLock lock(&_lock);

	ref<Node> nw = _network;
	if(!nw) return;
	InstanceID iid = nw->GetInstanceID();

	PacketHeader rph;
	rph._flags = PacketFlagRequestRedelivery;
	ref<Packet> rp = GC::Hold(new Packet(rph, (const char*)0, 0));
	std::deque< std::pair<InstanceID, ReliablePacketID> >::iterator it = _reliableWishList.begin();
	while(it!=_reliableWishList.end()) {
		rp->_header->_rpid = it->second;
		rp->_header->_from = iid;
		rp->_header->_plugin = it->first;
		///Log::Write(L"TJNP/Socket", L"Requesting redelivery (iid="+StringifyHex(it->first)+L" rpid="+Stringify(it->second)+L")");
		Send(rp, false);
		++it;
	}
}

void Socket::SendDemoted() {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionDemoted));
	Send(stream, true);
}

void Socket::SendError(Features fs, ExceptionType type, const std::wstring& msg) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionReportError));
	stream->Add(fs);
	stream->Add(type);
	stream->Add<std::wstring>(msg);
	Send(stream, true);
}

void Socket::SendAnnounce(Role r, const std::wstring& address, Features feats, strong<Transaction> ti) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionAnnounce));
	stream->Add(r);
	stream->Add(feats);
	
	// Since announce replies can either be handled by the default transaction (_network) or a separate transaction,
	// choose here.
	if(ref<Transaction>(ti)==ref<Transaction>(ref<Node>(_network))) {
		stream->Add<TransactionIdentifier>(0);
	}
	else {
		++_transactionCounter;
		_transactions[_transactionCounter] = ti;
		stream->Add<TransactionIdentifier>(_transactionCounter);
	}

	stream->Add<std::wstring>(address);
	Send(stream);
}

void Socket::SendAnnounceReply(Role r, const std::wstring& address, Features feats, TransactionIdentifier ti) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionAnnounceReply,ti));
	stream->Add(r);
	stream->Add(feats);
	stream->Add<std::wstring>(address);
	Send(stream);
}

void Socket::SendPromoted() {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionPromoted));
	Send(stream, true);
}

void Socket::SendResetAll() {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionResetAll));
	Send(stream, true);
}

void Socket::SendResetChannel(GroupID gid, Channel ch) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionResetChannel));
	stream->GetHeader()->_channel = ch;
	stream->GetHeader()->_group = gid;
	Send(stream, true);
}

void Socket::SendSetPatch(ref<BasicClient> c, const PatchIdentifier& pi, const DeviceIdentifier& di) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionSetPatch));
	stream->Add<InstanceID>(c->GetInstanceID());
	stream->Add<PatchIdentifier>(pi);
	stream->Add<DeviceIdentifier>(di);
	Send(stream, true);
}

void Socket::SendListPatchesReply(const PatchIdentifier& pi, const DeviceIdentifier& di, TransactionIdentifier ti, in_addr to, unsigned int count) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionListPatchesReply, ti));
	stream->Add<PatchIdentifier>(pi);
	stream->Add<DeviceIdentifier>(di);
	stream->Add<unsigned int>(count);
	Send(stream, true);
}

void Socket::SendListDevicesReply(const DeviceIdentifier& di, const std::wstring& friendly, TransactionIdentifier ti, in_addr to, unsigned int count) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionListDevicesReply, ti));
	stream->Add<DeviceIdentifier>(di);
	stream->Add<std::wstring>(friendly);
	stream->Add<unsigned int>(count);
	Send(stream, true);
}

void Socket::SendSetClientAddress(ref<BasicClient> client, std::wstring na) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(ActionSetAddress));
	stream->Add(client->GetInstanceID());
	stream->Add<std::wstring>(na);
	Send(stream, true);
}

void Socket::SendInput(const PatchIdentifier& patch, const InputID& path, float value) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(ActionInput));
	stream->Add(patch);
	stream->Add(path);
	stream->Add(value);
	Send(stream, true);
}

void Socket::SendListDevices(InstanceID to, ref<Transaction> ti) {
	ThreadLock lock(&_lock);

	if(ti) {
		++_transactionCounter;
		_transactions[_transactionCounter] = ti;

		ref<Message> msg = GC::Hold(new Message(ActionListDevices));
		msg->Add<InstanceID>(to);
		msg->Add<TransactionIdentifier>(_transactionCounter);
		Send(msg, true);
	}
}

void Socket::SendListPatches(InstanceID to, ref<Transaction> ti) {
	ThreadLock lock(&_lock);

	if(ti) {
		++_transactionCounter;
		_transactions[_transactionCounter] = ti;

		ref<Message> msg = GC::Hold(new Message(ActionListPatches));
		msg->Add<InstanceID>(to);
		msg->Add<TransactionIdentifier>(_transactionCounter);
		Send(msg, true);
	}
}

void Socket::SendLeave() {
	ThreadLock lock(&_lock);
	PacketHeader ph;
	ph._action = ActionLeave;
	ref<Packet> p = GC::Hold(new Packet(ph, (const char*)0, 0));
	Send(p, true);
}

void Socket::SendResourceFind(const std::wstring& ident, ref<Transaction> ti) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(ActionFindResource));
	stream->Add(ident);

	if(ti) {
		++_transactionCounter;
		_transactions[_transactionCounter] = ti;
		stream->Add<TransactionIdentifier>(_transactionCounter);
	}
	else {
		stream->Add<TransactionIdentifier>(0);
	}
	Send(stream);
}

void Socket::SendResourcePush(const GroupID& gid, const ResourceIdentifier& ident) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(ActionPushResource));
	stream->GetHeader()->_group = gid;
	stream->Add(ident);
	Send(stream, true);
}

void Socket::SendResourceAdvertise(const ResourceIdentifier& rid, const std::wstring& url, unsigned short port, TransactionIdentifier tid) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(ActionAdvertiseResource, tid));
	stream->Add(port);
	stream->Add(rid);
	stream->Add(url);
	Send(stream);
}

void Socket::SendOutletChange(Channel ch, GroupID gid, const std::wstring& outletName, const tj::shared::Any& value) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(ActionOutletChange));
	stream->Add<Channel>(ch);
	stream->Add<GroupID>(gid);
	stream->Add<unsigned int>(value.GetType());
	stream->Add(value.ToString());
	Hash hash;
	stream->Add<OutletHash>(hash.Calculate(outletName));
	Send(stream, true);
}

void Socket::Send(strong<Packet> p, bool reliable) {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)_port);      
	u_long dir_bcast_addr = inet_addr(_bcastAddress);
	addr.sin_addr.s_addr = dir_bcast_addr;
	Send(p, &addr, reliable);
}

ReliablePacketID Socket::RegisterReliablePacket(strong<Packet> p) {
	ThreadLock lock(&_lock);
	ReliablePacketID next = _lastPacketID + 1;

	if(_reliableSentPackets.size() > _maxReliablePacketCount) {
		/* Remove the entry with the id next - _maxReliablePacketCount. When next=101, we remove 101-100=1 */
		std::map<ReliablePacketID, ref<Packet> >::iterator it = _reliableSentPackets.find(next - _maxReliablePacketCount);
		if(it!=_reliableSentPackets.end()) {
			_reliableSentPackets.erase(it);
		}
		else {
			Log::Write(L"TJNP/Socket", L"Could not remove sent reliable packet from list; probably causing memory leaks!");
		}
	}

	if(_reliableSentPackets.size() > 2*_maxReliablePacketCount) {
		_reliableSentPackets.clear();
		Log::Write(L"TJNP/Socket", L"List of sent packets became too large; removed all of them!");
	}

	_reliableSentPackets[next] = p;
	p->_header->_flags |= PacketFlagReliable;
	p->_header->_rpid = next;
	_lastPacketID = next;
	return next;
}

void Socket::Send(strong<Packet> p, const sockaddr_in* address, bool reliable) {
	ThreadLock lock(&_lock);
	ref<Node> nw = _network;
	if(!nw) return;

	/* If packet needs to be sent 'reliably', create a ReliablePacketID */
	if(reliable) {
		RegisterReliablePacket(p);
		p->_header->_flags |= PacketFlagReliable;
	}

	/** For testing, drop a few packets **/
	///if(rand()%4 == 0) {
	///	Log::Write(L"TJNP/Socket", L"Deliberately dropping packet (rpid="+Stringify(p->_header->_rpid)+L") ");
	///	return;
	///}

	unsigned int size = min(Packet::maximumSize-sizeof(PacketHeader), p->GetSize()) + sizeof(PacketHeader);
	p->_header->_from = nw->GetInstanceID();
	int ret = sendto(_client, reinterpret_cast<char*>(p->_header), size, 0, (const sockaddr*)address, sizeof(sockaddr_in));

	if(ret != SOCKET_ERROR) {
		_bytesSent += (int)size;
	}
}

void Socket::Send(strong<Message> s, bool reliable) {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)_port);      
	u_long dir_bcast_addr = inet_addr(_bcastAddress);
	addr.sin_addr.s_addr = dir_bcast_addr;

	s->SetSent();
	Send(s->ConvertToPacket(), &addr, reliable);
}

unsigned int Socket::GetWishListSize() const {
	return (unsigned int)_reliableWishList.size();
}

#ifdef TJ_OS_WIN
	LRESULT CALLBACK SocketMessageWindowProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
		if(msg==WM_CREATE) {
			return 1;
		}
		else if(msg==TJSOCKET_MESSAGE) {
			Socket* sock = reinterpret_cast<Socket*>((long long)GetWindowLong(wnd, GWL_USERDATA));
			if(sock) sock->Receive();
		}

		return DefWindowProc(wnd,msg,wp,lp);
	}
#endif

int Socket::GetBytesSent() const {
	return _bytesSent;
}

int Socket::GetBytesReceived() const {
	return _bytesReceived;
}

NetworkInitializer::NetworkInitializer() {
	#ifdef TJ_OS_WIN
		_data = 0;
	#endif
}

void NetworkInitializer::Initialize() {
	#ifdef TJ_OS_WIN
		if(InterlockedExchange((volatile long*)&_data, 1)==0) {
			_data = (void*)new WSADATA();

			WNDCLASS wc;
			memset(&wc,0,sizeof(WNDCLASS));
			wc.hInstance = GetModuleHandle(NULL);
			wc.lpszClassName = TJSOCKET_MESSAGE_CLASS;
			wc.lpfnWndProc = SocketMessageWindowProc;
			
			if(!RegisterClass(&wc)) {
				Throw(L"Could not register socket message listener class", ExceptionTypeError);
			}
			
			if(WSAStartup(MAKEWORD(1,1), (WSADATA*)&_data)!=0) {
				Throw(L"WSAStartup failed, usually means your computer doesn't have a network or something is really wrong.", ExceptionTypeError);	
			}
		}
	#endif
}

NetworkInitializer::~NetworkInitializer() {
	// This causes crash? WSAShutdown is not needed (it doesn't exist) anyway.,..
	/*if(_data!=0) {
		delete _data;
		_data = 0;
	}*/
}

#pragma pack(pop)