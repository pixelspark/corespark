#include "../include/tjnp.h"

#include <time.h>
#include <sstream>

using namespace tj::shared;
using namespace tj::np;

#pragma pack(push,1)

LRESULT CALLBACK SocketMessageWindowProc(HWND, UINT, WPARAM, LPARAM);
#define TJSOCKET_MESSAGE_CLASS (L"TjSocketMessageWnd")
#define TJSOCKET_MESSAGE (WM_USER+1338)

NetworkInitializer Socket::_initializer;

Socket::Socket(int port, const char* address, ref<Node> nw) {
	_bytesSent = 0;
	_bytesReceived = 0;
	_network = nw;

	// Create a random transaction counter id
	_transactionCounter = rand();

	_initializer.Initialize();
	assert(address!=0 && port > 0 && port < 65536);
	_recieveBuffer = new char[Packet::maximumSize];

	_port = port;
	_bcastAddress = _strdup(address);
	
	_window = CreateWindow(TJSOCKET_MESSAGE_CLASS, L"SocketWnd", 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0);
	if(!_window) {
		Throw(L"Couldn't create message window for socket.", ExceptionTypeError);
	}
	SetWindowLong(_window, GWL_USERDATA, LONG((long long)this));

	SOCKADDR_IN addr;
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

	WSAAsyncSelect(_server,_window,TJSOCKET_MESSAGE,FD_READ);

	_client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(_client == INVALID_SOCKET) {
		Throw(L"Couldn't open socket for broadcasting",ExceptionTypeError);
		return;
	}

	setsockopt(_client,SOL_SOCKET,SO_BROADCAST,(const char*)&on, sizeof(int));
	setsockopt(_client,SOL_SOCKET,SO_REUSEADDR,(const char*)&on, sizeof(int));
	//ioctlsocket(_client, FIONBIO, (u_long*)1);
}

Socket::~Socket() {
	closesocket(_client);
	DestroyWindow(_window);
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
		int size = (int)sizeof(from);
		int ret = recvfrom(_server, _recieveBuffer, Packet::maximumSize-1, 0, (sockaddr*)&from, &size);
		WSAAsyncSelect(_server,_window,TJSOCKET_MESSAGE,FD_READ);
		if(ret == SOCKET_ERROR) {
			// This seems to happen on packets that come from us
			return;
		}
		_bytesReceived += ret;

		// Extract packet header
		ph = *((PacketHeader*)_recieveBuffer);
		
		// Check if this actually is a TNP3 packet
		if(ph._version[0]!='T' || ph._version[1] != 'N' || ph._version[2]!='P' || ph._version[3]!='3') {
			Log::Write(L"TJNP/Socket", L"Received invalid packets from the network; maybe network link is broken or other applications are running on this port");
			return;
		}

		// Check size
		if(int(ph._size+sizeof(PacketHeader))>ret) {
			Log::Write(L"TJNP/Socket", L"Packet smaller than it says it is");
			return;
		}

		// Extract packet contents
		code = GC::Hold(new Code(_recieveBuffer+sizeof(PacketHeader), ph._size));

		// Find our transaction, if it exists. Otherwise, use the 'default transaction' (which happens to be _network)
		if(ph._transaction==0) {
			tx = ref<Node>(_network);
			if(!tx) {
				Log::Write(L"TJNP/Socket", L"Network instance was deleted");
			}
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

void Socket::SendDemoted() {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->GetHeader()->_action = ActionDemoted;
	Send(stream);
}

void Socket::SendError(Features fs, ExceptionType type, const std::wstring& msg) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->Add(fs);
	stream->Add(type);
	stream->Add<std::wstring>(msg);
	stream->GetHeader()->_action = ActionReportError;
	Send(stream);
}

void Socket::SendAnnounce(Role r, const std::wstring& address, Features feats, strong<Transaction> ti) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->GetHeader()->_action = ActionAnnounce;
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

	ref<Message> stream = GC::Hold(new Message(true));
	stream->Add(r);
	stream->Add(feats);
	stream->Add<std::wstring>(address);
	stream->GetHeader()->_action = ActionAnnounceReply;
	stream->GetHeader()->_transaction = ti;
	Send(stream);
}

void Socket::SendPromoted() {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->GetHeader()->_action = ActionPromoted;
	Send(stream);
}

void Socket::SendResetAll() {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->GetHeader()->_action = ActionResetAll;
	Send(stream);
}

void Socket::SendSetPatch(ref<BasicClient> c, const PatchIdentifier& pi, const DeviceIdentifier& di) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->GetHeader()->_action = ActionSetPatch;
	stream->Add<InstanceID>(c->GetInstanceID());
	stream->Add<PatchIdentifier>(pi);
	stream->Add<DeviceIdentifier>(di);
	Send(stream);
}

void Socket::SendListPatchesReply(const PatchIdentifier& pi, const DeviceIdentifier& di, TransactionIdentifier ti, in_addr to, bool isLast) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->Add<PatchIdentifier>(pi);
	stream->Add<DeviceIdentifier>(di);
	stream->Add<bool>(isLast);
	stream->GetHeader()->_action = ActionListPatchesReply;
	stream->GetHeader()->_transaction = ti;
	Send(stream);
}

void Socket::SendListDevicesReply(const DeviceIdentifier& di, const std::wstring& friendly, TransactionIdentifier ti, in_addr to, bool isLast) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->Add<DeviceIdentifier>(di);
	stream->Add<std::wstring>(friendly);
	stream->Add<bool>(isLast);
	stream->GetHeader()->_action = ActionListDevicesReply;
	stream->GetHeader()->_transaction = ti;
	Send(stream);
}

void Socket::SendSetClientAddress(ref<BasicClient> client, std::wstring na) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(true));
	stream->Add(client->GetInstanceID());
	stream->Add<std::wstring>(na);
	stream->GetHeader()->_action = ActionSetAddress;
	Send(stream);
}

void Socket::SendInput(const PatchIdentifier& patch, const ChannelID& cid, const SubChannelID& scid, float value) {
	ThreadLock lock(&_lock);

	ref<Message> stream = GC::Hold(new Message(true));
	stream->GetHeader()->_action = ActionInput;
	stream->Add(patch);
	stream->Add(cid);
	stream->Add(scid);
	stream->Add(value);
	Send(stream);
}

void Socket::SendListDevices(InstanceID to, ref<Transaction> ti) {
	ThreadLock lock(&_lock);

	if(ti) {
		++_transactionCounter;
		_transactions[_transactionCounter] = ti;

		ref<Message> msg = GC::Hold(new Message(true));
		msg->Add<InstanceID>(to);
		msg->Add<TransactionIdentifier>(_transactionCounter);
		msg->GetHeader()->_action = ActionListDevices;
		Send(msg);
	}
}

void Socket::SendListPatches(InstanceID to, ref<Transaction> ti) {
	ThreadLock lock(&_lock);

	if(ti) {
		++_transactionCounter;
		_transactions[_transactionCounter] = ti;

		ref<Message> msg = GC::Hold(new Message(true));
		msg->Add<InstanceID>(to);
		msg->Add<TransactionIdentifier>(_transactionCounter);
		msg->GetHeader()->_action = ActionListPatches;
		Send(msg);
	}
}

void Socket::SendLeave() {
	ThreadLock lock(&_lock);
	PacketHeader ph;
	ph._action = ActionLeave;
	ref<Packet> p = GC::Hold(new Packet(ph, 0, 0));
	Send(p);
}

void Socket::SendResourceFind(const std::wstring& ident, ref<Transaction> ti) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(true));
	stream->Add(ident);

	if(ti) {
		++_transactionCounter;
		_transactions[_transactionCounter] = ti;
		stream->Add<TransactionIdentifier>(_transactionCounter);
	}
	else {
		stream->Add<TransactionIdentifier>(0);
	}

	stream->GetHeader()->_action = ActionFindResource;
	Send(stream);
}

void Socket::SendResourcePush(Channel c, const ResourceIdentifier& ident) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(true));
	stream->GetHeader()->_action = ActionPushResource;
	stream->GetHeader()->_channel = c;
	stream->Add(ident);
	Send(stream);
}

void Socket::SendResourceAdvertise(const ResourceIdentifier& rid, const std::wstring& url, unsigned short port, TransactionIdentifier tid) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(true));

	stream->GetHeader()->_channel = 0;
	stream->GetHeader()->_transaction = tid;
	stream->GetHeader()->_action = ActionAdvertiseResource;
	stream->Add(port);
	stream->Add(rid);
	stream->Add(url);
	Send(stream);
}

void Socket::SendOutletChange(Channel ch, const std::wstring& outletName, const tj::shared::Any& value) {
	ThreadLock lock(&_lock);
	ref<Message> stream = GC::Hold(new Message(true));
	stream->GetHeader()->_channel = 0;
	stream->GetHeader()->_action = ActionOutletChange;

	stream->Add<Channel>(ch);
	stream->Add<short>(value.GetType());
	stream->Add(value.ToString());
	Hash hash;
	stream->Add<OutletHash>(hash.Calculate(outletName));
	Send(stream);
}

void Socket::Send(ref<Packet> p) {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)_port);      
	u_long dir_bcast_addr = inet_addr(_bcastAddress);
	addr.sin_addr.s_addr = dir_bcast_addr;

	Send(p, &addr);
}

void Socket::Send(ref<Packet> p, const sockaddr_in* address) {
	ThreadLock lock(&_lock);
	ref<Node> nw = _network;
	if(!nw) return;

	/* Compose packet */
	memset(_recieveBuffer,0,Packet::maximumSize);
	PacketHeader* bufferPH = (PacketHeader*)_recieveBuffer;
	*bufferPH = p->GetHeader();
	bufferPH->_from = nw->GetInstanceID();

	if(p->GetSize()>0) {
		char* bufferMessage = (char*)(_recieveBuffer + sizeof(PacketHeader));
		memcpy(bufferMessage,p->GetMessage(), min(Packet::maximumSize-sizeof(PacketHeader), p->GetSize()));
	}

	unsigned int size = min(Packet::maximumSize-sizeof(PacketHeader), p->GetSize()) + sizeof(PacketHeader);
	int ret = sendto(_client,_recieveBuffer,size, 0,(const sockaddr*)address,sizeof(sockaddr_in));

	if(ret != SOCKET_ERROR) {
		_bytesSent += (int)size;
	}
}

void Socket::Send(ref<Message> s) {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)_port);      
	u_long dir_bcast_addr = inet_addr(_bcastAddress);
	addr.sin_addr.s_addr = dir_bcast_addr;

	Send(s,&addr);
}

void Socket::Send(ref<Message> s, const sockaddr_in* address) {
	if(!s) return;
	ref<Node> nw = _network;
	if(!nw) return;
	s->GetHeader()->_from = nw->GetInstanceID();

	ThreadLock lock(&_lock);

	unsigned int size = s->GetSize();
	int ret = sendto(_client,s->GetBuffer(),size, 0,(const sockaddr*)address,sizeof(sockaddr_in));

	if(ret == SOCKET_ERROR) {
		// oops...
		std::string a = inet_ntoa(*((const in_addr*)address));
		Log::Write(L"TJNP/Socket", L"Could not send client-specific message, address was " +Wcs(a));
	}
	else {
		_bytesSent += (int)size;
	}
}

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

int Socket::GetBytesSent() const {
	return _bytesSent;
}

int Socket::GetBytesReceived() const {
	return _bytesReceived;
}

NetworkInitializer::NetworkInitializer() {
	_data = 0;
}

void NetworkInitializer::Initialize() {
	ThreadLock lock(&_lock);

	if(_data==0) {
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
}

NetworkInitializer::~NetworkInitializer() {
	// This causes crash? WSAShutdown is not needed (it doesn't exist) anyway.,..
	/*if(_data!=0) {
		delete _data;
		_data = 0;
	}*/
}

#pragma pack(pop)