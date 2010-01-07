#include "../include/tjsocket.h"

#include <time.h>
#include <sstream>
#include <limits>
#include <errno.h>

#ifndef TJ_OS_WIN
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define _strdup strdup
#else
	typedef int socklen_t;

	namespace tj {
		namespace np {
			LRESULT CALLBACK SocketListenerWindowProc(HWND, UINT, WPARAM, LPARAM);
		}
	}

	#define TJSOCKET_MESSAGE_CLASS (L"TjSocketMessageWnd")
	#define TJSOCKET_MESSAGE (WM_USER+1338)
#endif

#ifdef TJ_OS_POSIX
	#include <fcntl.h>
#endif

using namespace tj::shared;
using namespace tj::np;

#ifdef TJ_OS_WIN
	void* NetworkInitializer::_data = 0;
#endif

NetworkInitializer::NetworkInitializer() {
	Initialize();
}

void NetworkInitializer::Initialize() {
#ifdef TJ_OS_WIN
	if(InterlockedExchange((volatile long*)&_data, 1)==0) {
		_data = (void*)new WSADATA();
		
		WNDCLASS wc;
		memset(&wc,0,sizeof(WNDCLASS));
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = TJSOCKET_MESSAGE_CLASS;
		wc.lpfnWndProc = SocketListenerWindowProc;
		
		if(!RegisterClass(&wc)) {
			Throw(L"Could not register socket message listener class", ExceptionTypeError);
		}
		
		if(WSAStartup(MAKEWORD(2,2), (WSADATA*)&_data)!=0) {
			Throw(L"WSAStartup failed, usually means your computer doesn't have a network or something is really wrong.", ExceptionTypeError);	
		}
	}
#endif
}

NetworkInitializer::~NetworkInitializer() {
}

/** Socket **/
#ifdef TJ_OS_WIN
	const NativeSocket Socket::KInvalidSocket = INVALID_SOCKET;
#endif

#ifdef TJ_OS_POSIX
	const NativeSocket Socket::KInvalidSocket = -1;
#endif

Socket::Socket(NativeSocket ns): _socket(ns) {
}

Socket::Socket(const NetworkAddress& ns, TransportProtocol tp, unsigned short port) {
	if(!Create(ns.GetAddressFamily(), tp)) {
		Throw(L"Could not create socket", ExceptionTypeError);
	}

	if(!Connect(ns,port)) {
		Log::Write(L"TJNP/Socket", L"Could not connect TCP socket; address="+ns.ToString()+L" port="+Stringify(port)+L" error="+Util::GetDescriptionOfSystemError(errno));
		Close();
		Throw(L"Could not connect socket", ExceptionTypeError);
	}
}

Socket::Socket(AddressFamily af, TransportProtocol tp) {
	if(!Create(af,tp)) {
		Throw(L"Could not create socket", ExceptionTypeError);
	}
}

void Socket::Close() {
	#ifdef TJ_OS_WIN
		shutdown(_socket, SD_BOTH);
		closesocket(_socket);
	#endif

	#ifdef TJ_OS_POSIX
		shutdown(_socket, SHUT_RDWR);
		close(_socket);
	#endif

	_socket = KInvalidSocket;
}

bool Socket::IsValid() const {
	return _socket != KInvalidSocket;
}

bool Socket::Read(char* buffer, unsigned int length, unsigned int& readBytes) {
	int r = recv(_socket, buffer, length*sizeof(char), 0);
	if(r>0) {
		readBytes = r;
		return true;
	}
	else if(r==0) {
		Close();
	}
	readBytes = 0;
	return false;
}

bool Socket::SendTo(const NetworkAddress& na, unsigned short port, strong<Data> data) {
	return SendTo(na, port, data->GetBuffer(), data->GetSize());
}

bool Socket::SendTo(const NetworkAddress& na, unsigned short port, const std::string& data) {
	return SendTo(na, port, data.c_str(), (int)data.length()*sizeof(char));
}

bool Socket::SendTo(const NetworkAddress& na, unsigned short port, const char* data, const Bytes& length) {
	switch(na.GetAddressFamily()) {
		case AddressFamilyIPv4: {
			sockaddr_in addr;
			if(na.GetIPv4SocketAddress(&addr)) {
				addr.sin_port = htons(port);
				if(sendto(_socket, data, (int)length, 0, (const sockaddr*)&addr, sizeof(sockaddr_in))<=0) {
					return false;
				}
				return true;
			}
			break;
		}

		case AddressFamilyIPv6: {
			sockaddr_in6 addr;
			if(na.GetIPv6SocketAddress(&addr)) {
				addr.sin6_port = htons(port);
				if(sendto(_socket, data, (int)length, 0, (const sockaddr*)&addr, sizeof(sockaddr_in6))<=0) {
					return false;
				}
				return true;
			}
			break;
		}

		default:
			Throw(L"Socket::SendTo: unsupported address family!", ExceptionTypeError);
	};

	return false;
}

bool Socket::Send(strong<Data> data) {
	return Send(data->GetBuffer(), data->GetSize());
}

bool Socket::Send(const std::string& data) {
	return Send(data.c_str(), (Bytes)data.length());
}

bool Socket::Send(const char* data, const Bytes& length) {
	if(send(_socket, data, (int)length, 0)<=0) {
		return false;
	}
	return true;
}

bool Socket::Connect(const NetworkAddress& networkAddress, unsigned short port) {
	void* toAddress = 0;
	unsigned int toAddressSize = 0;
	sockaddr_in6 addr6;
	sockaddr_in addr4;
	
	if(networkAddress.GetAddressFamily()==AddressFamilyIPv6) {
		networkAddress.GetIPv6SocketAddress(&addr6);
		addr6.sin6_port = htons(port);
		toAddress = reinterpret_cast<void*>(&addr6);
		toAddressSize = sizeof(sockaddr_in6);
	}
	else if(networkAddress.GetAddressFamily()==AddressFamilyIPv4) {
		networkAddress.GetIPv4SocketAddress(&addr4);
		addr4.sin_port = htons(port);
		toAddress = reinterpret_cast<void*>(&addr4);
		toAddressSize = sizeof(sockaddr_in);
	}
	else {
		Throw(L"Socket::SendTo: unsupported address family!", ExceptionTypeError);
	}

	if(connect(_socket, (const sockaddr*)toAddress, toAddressSize)!=0) {
		return false;
	}
	return true;
}

bool Socket::Create(AddressFamily af, TransportProtocol tp) {
	int pf = 0;
	switch(af) {
		case AddressFamilyIPv4:
			pf = PF_INET;
			break;

		case AddressFamilyIPv6:
			pf = PF_INET6;
			break;
	}

	int type = 0;
	int proto = 0;
	switch(tp) {
		case TransportProtocolTCP:
			type = SOCK_STREAM;
			proto = IPPROTO_TCP;
			break;

		case TransportProtocolUDP:
			type = SOCK_DGRAM;
			proto = IPPROTO_UDP;
			break;
	}

	_socket = socket(pf,type,proto);
	if(_socket==-1) {
		return false;
	}
	return true;
}

void Socket::SetBroadcast(bool b) {
	int on = b ? 1 : 0;
	setsockopt(_socket, SOL_SOCKET, SO_BROADCAST,(const char*)&on, sizeof(int));
}

void Socket::SetReuseAddress(bool b) {
	int on = b ? 1 : 0;

	#ifdef TJ_OS_MAC
		setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, (const char*)&on, sizeof(int));
	#else
		setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
	#endif
}

void Socket::SetBlocking(bool b) {
	#ifdef TJ_OS_POSIX
		if(b) {
			fcntl(_socket, F_SETFL, O_NONBLOCK);
		}
		else {
			fcntl(_socket, F_SETFL, 0);
		}
	#endif
	
	#ifdef TJ_OS_WIN
		unsigned long onl = b ? 1 : 0;
		ioctlsocket(_socket, FIONBIO, &onl);
	#endif
}

NativeSocket Socket::GetNativeSocket() const {
	return _socket;
}

Socket::~Socket() {
	Close();
}

/** SocketListener **/
SocketListener::~SocketListener() {
}

/** SocketListenerThread **/

/* The process wide instance of SocketListenerThread. Note that it is stored as a
 weak reference, so that it is destroyed when it is not used anymore */
weak<SocketListenerThread> SocketListenerThread::_instance;

strong<SocketListenerThread> SocketListenerThread::DefaultInstance() {
	ref<SocketListenerThread> slt = _instance;
	if(!slt) {
		slt = GC::Hold(new SocketListenerThread());
		_instance = slt;
		slt->Start();
	}
	return slt;
}

SocketListenerThread::SocketListenerThread() {
	#ifdef TJ_OS_POSIX
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, _controlSocket)!=0) {
			Log::Write(L"TJNP/SocketListenerThread", L"Could not create control socket pair");
		}
	#endif
}

SocketListenerThread::~SocketListenerThread() {
	{
		ThreadLock lock(&_lock);
		Stop();
	}
	
	WaitForCompletion();
	
	#ifdef TJ_OS_POSIX
		close(_controlSocket[0]);
		close(_controlSocket[1]);
	#endif
		
	#ifdef TJ_OS_WIN
		DestroyWindow(_window);
	#endif
}

void SocketListenerThread::PostThreadUpdate() {
	#ifdef TJ_OS_WIN
		PostThreadMessage(GetID(), WM_USER, 0, 0);
	#endif
		
	#ifdef TJ_OS_POSIX
		char update[1] = {'U'};
		if(write(_controlSocket[0], update, 1)==-1) {
			Log::Write(L"TJNP/SocketListenerThread", L"Could not send update message to listener thread");
		}
	#endif
}

void SocketListenerThread::RemoveListener(NativeSocket ns) {
	{
		ThreadLock lock(&_lock);
		std::map<NativeSocket, weak<SocketListener> >::iterator it = _listeners.find(ns);
		if(it!=_listeners.end()) {
			_listeners.erase(it);
		}
	}

	PostThreadUpdate();
}

void SocketListenerThread::AddListener(NativeSocket sock, ref<SocketListener> sl) {
	{
		ThreadLock lock(&_lock);
		_listeners[sock] = sl;
	}
	PostThreadUpdate();
}

void SocketListenerThread::Stop() {
	#ifdef TJ_OS_WIN
		PostThreadMessage(GetID(), WM_QUIT, 0, 0);
	#endif
		
	#ifdef TJ_OS_POSIX
		char quit[1] = {'Q'};
		if(write(_controlSocket[0], quit, 1)==-1) {
			Log::Write(L"TJNP/SocketListenerThread", L"Could not send quit message to listener thread");
		}
	#endif
}

void SocketListenerThread::OnReceive(NativeSocket ns) {
	ref<SocketListener> sl;
	{
		ThreadLock lock(&_lock);
		sl = _listeners[ns];
	}
	
	if(sl) {
		try {
			sl->OnReceive(ns);
		}
		catch(const Exception& e) {
			Log::Write(L"TJNP/SocketListenerThread", L"Exception occurred when accepting connection or receiving data from socket "+StringifyHex(ns)+L": "+e.GetMsg());
		}
		catch(...) {
			Log::Write(L"TJNP/SocketListenerThread", L"Unknown exception occurred when accepting connection or receiving data from socket "+StringifyHex(ns));

		}
	}
	
	#ifdef TJ_OS_WIN
		// Restart the asynchronous select, since it stops whenever the socket is readable
		WSAAsyncSelect(ns, _window, TJSOCKET_MESSAGE, FD_READ|FD_ACCEPT);
	#endif
}

void SocketListenerThread::Run() {
	#ifdef TJ_OS_WIN
		_window = CreateWindow(TJSOCKET_MESSAGE_CLASS, L"SocketWnd", 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0);
		if(!_window) {
			DWORD gle = GetLastError();
			Log::Write(L"TJNP/SocketListenerThread", L"Could not create message window; error="+Stringify(gle));
			Throw(L"Couldn't create message window for socket.", ExceptionTypeError);
		}
		SetWindowLong(_window, GWL_USERDATA, LONG((long long)this));
	
		bool running = true;
		bool updateSelect = true;
	
		while(running) {
			if(updateSelect) {
				updateSelect = false;
				ThreadLock lock(&_lock);
				std::map<NativeSocket, weak<SocketListener> >::iterator it = _listeners.begin();
				while(it!=_listeners.end()) {
					WSAAsyncSelect(it->first, _window, TJSOCKET_MESSAGE, FD_READ|FD_ACCEPT);
					++it;
				}
			}
			
			MSG msg;
			GetMessage(&msg,0,0,0);
				
			if(msg.message==WM_USER) {
				updateSelect = true;
				continue;
			}
			else if(msg.message==WM_QUIT) {
				running = false;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		DestroyWindow(_window);
	#endif	
	
	#ifdef TJ_OS_POSIX
		while(true) {
			fd_set fds;
			FD_ZERO(&fds);
			int maxSocket = _controlSocket[1];
			FD_SET(_controlSocket[1], &fds);
			
			{
				ThreadLock lock(&_lock);
				std::map<NativeSocket, weak<SocketListener> >::iterator it = _listeners.begin();
				while(it!=_listeners.end()) {
					if(it->first!=-1) {
						FD_SET(it->first, &fds);
						maxSocket = Util::Max(maxSocket, it->first);
					}
					++it;
				}
			}

			if(select(maxSocket+1, &fds, NULL, NULL, NULL)>0) {
				if(FD_ISSET(_controlSocket[1], &fds)) {
					char cmd = 'Q';
					recv(_controlSocket[1], &cmd, sizeof(char), 0);
					
					if(cmd=='Q') {
						// End the thread, a control message was sent to us
						Log::Write(L"TJNP/SocketListenerThread", L"End thread, quit control message received");
						return;
					}
					else if(cmd=='U') {
						// Update, restart select()
						continue;
					}
					else {
						Log::Write(L"TJNP/SocketListenerThread", L"Unknown command");
					}
				}
				else {
					/* This weird way of doing is to make sure that the socket listener thread is not locked
					whenever listener->OnReceive is called. */
					std::set<NativeSocket> readySockets;
					{
						ThreadLock lock(&_lock);
						std::map<NativeSocket, weak<SocketListener> >::iterator it = _listeners.begin();
						while(it!=_listeners.end()) {
							if(it->first!=-1) {
								if(FD_ISSET(it->first, &fds)) {
									readySockets.insert(it->first);
								}
							}
							++it;
						}
					}
					
					std::set<NativeSocket>::iterator sit = readySockets.begin();
					while(sit!=readySockets.end()) {
						OnReceive(*sit);
						++sit;
					}
				}
			}
			else {
				Log::Write(L"TJNP/SocketListenerThread", L"Select operation failed; closing socket? Terminating listener thread!");
				return;
			}
		}
	#endif

	Log::Write(L"TJNP/SocketListenerThread", L"End socket listener thread");
}

#ifdef TJ_OS_WIN
	namespace tj {
		namespace np {
			LRESULT CALLBACK SocketListenerWindowProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
				if(msg==WM_CREATE) {
					return 1;
				}
				else if(msg==TJSOCKET_MESSAGE) {
					SocketListenerThread* sock = reinterpret_cast<SocketListenerThread*>((long long)GetWindowLong(wnd, GWL_USERDATA));
					if(sock) sock->OnReceive(wp);
				}
				
				return DefWindowProc(wnd,msg,wp,lp);
			}
		}
	}
#endif
