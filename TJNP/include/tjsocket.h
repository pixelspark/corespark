#ifndef _TJ_NP_SOCKET_UTIL_H
#define _TJ_NP_SOCKET_UTIL_H

#include "internal/tjnp.h"
#include "tjnetworkaddress.h"

struct sockaddr_in;
struct sockaddr_in6;

#pragma warning(push)
#pragma warning(disable: 4251 4275)

namespace tj {
	namespace np {
		class NP_EXPORTED NetworkInitializer {
			public:
				NetworkInitializer();
				~NetworkInitializer();
				void Initialize();

			protected:
				#ifdef TJ_OS_WIN
					static void* _data;
				#endif
		};

		#ifdef TJ_OS_WIN
			typedef SOCKET NativeSocket;
		#else
			typedef int NativeSocket;
		#endif

		class NP_EXPORTED Socket: public virtual tj::shared::Object {
			public:
				Socket(NativeSocket ns);
				Socket(const NetworkAddress& ad, TransportProtocol tp, unsigned short port);
				Socket(AddressFamily fm, TransportProtocol tp);
				virtual ~Socket();
				virtual bool Connect(const NetworkAddress& ns, unsigned short port);
				virtual void Close();
				virtual bool IsValid() const;
				virtual NativeSocket GetNativeSocket() const;
				virtual bool Send(const std::string& data);
				virtual bool Read(char* buffer, unsigned int maxLength, unsigned int& readBytes);

			protected:
				const static NativeSocket KInvalidSocket;
				virtual bool Create(AddressFamily fm, TransportProtocol tp);
				NativeSocket _socket;
		};

		class NP_EXPORTED SocketListener: public virtual tj::shared::Object {
			public:
				virtual ~SocketListener();
				virtual void OnReceive(NativeSocket ns) = 0;
		};
		
		class NP_EXPORTED SocketListenerThread: public tj::shared::Thread {
			#ifdef TJ_OS_WIN
				friend LRESULT CALLBACK SocketListenerWindowProc(HWND, UINT, WPARAM, LPARAM);
			#endif
						
			public:
				SocketListenerThread();
				virtual ~SocketListenerThread();
				virtual void AddListener(NativeSocket sock, tj::shared::ref<SocketListener> sl);
				virtual void RemoveListener(NativeSocket sock);
				virtual void Run();
				virtual void Stop();
			
			protected:
				virtual void OnReceive(NativeSocket ns);
				
				tj::shared::CriticalSection _lock;
				std::map<NativeSocket, tj::shared::weak<SocketListener> > _listeners;
				
			private:
				NetworkInitializer _ni;
				virtual void PostThreadUpdate();
			
				#ifdef TJ_OS_POSIX
					NativeSocket _controlSocket[2];
				#endif
							
				#ifdef TJ_OS_WIN
					HWND _window;
				#endif
		};
	}
}

#pragma warning(pop)

#endif