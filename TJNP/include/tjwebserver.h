#ifndef _TJ_NP_WEBSERVER_H
#define _TJ_NP_WEBSERVER_H

#include "internal/tjnp.h"
#include "tjsocket.h"
#include "tjhttp.h"

#pragma warning(push)
#pragma warning(disable:4251 4275)

namespace tj {
	namespace np {
		class WebServer;

		class NP_EXPORTED WebServerThread: public tj::shared::Thread {
			public:
				WebServerThread(tj::shared::ref<WebServer> fs, int port);
				virtual ~WebServerThread();
				virtual void Run();
				virtual void Cancel();
				virtual void Start();
				virtual unsigned short GetActualPort() const;

			protected:
				tj::shared::weak<WebServer> _fs;
				tj::shared::Event _readyEvent;
				int _port;
				NativeSocket _server4, _server6;
			
				#ifdef TJ_OS_POSIX
					NativeSocket _controlSocket[2];
				#endif
		};

		class NP_EXPORTED WebServerResponseThread: public tj::shared::Thread {
			public:
				WebServerResponseThread(NativeSocket client, tj::shared::ref<WebServer> fs);
				virtual ~WebServerResponseThread();
				virtual void SendError(int code, const tj::shared::String& desc, const tj::shared::String& extraInfo);
				virtual void ServePage(tj::shared::ref<HTTPRequest> hrp);
				virtual void Run();

			protected:
				NativeSocket _client;
				tj::shared::weak<WebServer> _fs;
		};


		class NP_EXPORTED WebServer: public virtual tj::shared::Object {
			friend class WebServerThread;
			friend class WebServerResponseThread;

			public:
				WebServer(unsigned short port, tj::shared::ref<FileRequestResolver> defaultResolver = tj::shared::null);
				virtual ~WebServer();
				virtual void OnCreated();
				virtual unsigned int GetBytesReceived() const;
				virtual unsigned int GetBytesSent() const;
				virtual void Stop();
				virtual unsigned short GetActualPort() const;
				virtual void AddResolver(const tj::shared::String& pathPrefix, tj::shared::strong<FileRequestResolver> fr);

				const static unsigned short KPortDontCare = 0;
			
			protected:
				tj::shared::CriticalSection _lock;
				std::map< tj::shared::String, tj::shared::ref<FileRequestResolver> > _resolvers;
				tj::shared::ref<FileRequestResolver> _defaultResolver;
				tj::shared::ref<WebServerThread> _serverThread;
				volatile bool _run;
				unsigned int _bytesReceived;
				unsigned int _bytesSent;
				unsigned short _port;
				NetworkInitializer _initializer;
		};

	}
}

#pragma warning(pop)
#endif