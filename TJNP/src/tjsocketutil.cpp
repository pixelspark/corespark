#include "../include/tjsocketutil.h"

#include <time.h>
#include <sstream>
#include <limits>

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

using namespace tj::shared;
using namespace tj::np;

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
		wc.lpfnWndProc = SocketListenerWindowProc;
		
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

/** SocketListener **/
SocketListener::~SocketListener() {
}


/** SocketListenerThread **/
SocketListenerThread::SocketListenerThread() {
	#ifdef TJ_OS_POSIX
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, _controlSocket)!=0) {
			Log::Write(L"TJNP/SocketListenerThread", L"Could not create control socket pair");
		}
	#endif
}

SocketListenerThread::~SocketListenerThread() {
	Stop();
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
	ThreadLock lock(&_lock);
	ref<SocketListener> sl = _listeners[ns];
	if(sl) {
		sl->OnReceive(ns);
	}
	
	#ifdef TJ_OS_WIN
		// Restart the asynchronous select, since it stops whenever the socket is readable
		WSAAsyncSelect(ns, _window, TJSOCKET_MESSAGE, FD_READ);
	#endif
}

void SocketListenerThread::Run() {
	#ifdef TJ_OS_WIN
		_window = CreateWindow(TJSOCKET_MESSAGE_CLASS, L"SocketWnd", 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0);
		if(!_window) {
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
					WSAAsyncSelect(it->first, _window, TJSOCKET_MESSAGE, FD_READ);
					++it;
				}
			}
			
			MSG msg;
			GetMessage(&msg,0,0,0)
				
			if(msg.wMsg==WM_USER) {
				updateSelect = true;
				break;
			}
			else if(msg.wMsg==WM_QUIT) {
				running = false;
				break;
			}
		}
		
		DestroyWindow(_window);
	#endif	
	
	#ifdef TJ_OS_POSIX
		while(true) {
			fd_set fds;
			fd_set fdsErrors;
			FD_ZERO(&fds);
			int maxSocket = _controlSocket[1];
			FD_SET(_controlSocket[1], &fds);
			FD_SET(_controlSocket[1], &fdsErrors);
			
			{
				ThreadLock lock(&_lock);
				std::map<NativeSocket, weak<SocketListener> >::iterator it = _listeners.begin();
				while(it!=_listeners.end()) {
					if(ref<SocketListener>(it->second)) {
						if(it->first!=-1) {
							FD_SET(it->first, &fds);
							FD_SET(it->first, &fdsErrors);
							maxSocket = max(maxSocket, it->first);
						}
					}
					++it;
				}
			}
			
			if(select(maxSocket+1, &fds, NULL, &fdsErrors, NULL)>0) {
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
					ThreadLock lock(&_lock);
					std::map<NativeSocket, weak<SocketListener> >::iterator it = _listeners.begin();
					while(it!=_listeners.end()) {
						if(it->first!=-1) {
							if(FD_ISSET(it->first, &fdsErrors)) {
								Log::Write(L"TJNP/SocketListenerThread", L"Error on socket "+StringifyHex(it->first));
							}
							else if(FD_ISSET(it->first, &fds)) {
								OnReceive(it->first);
							}
						}
						++it;
					}
				}
			}
			else {
				Log::Write(L"TJNP/SocketListenerThread", L"Select operation failed; closing socket? Terminating listener thread!");
				return;
			}
		}
	#endif
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
