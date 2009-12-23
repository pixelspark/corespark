#ifndef _TJ_NP_WEBSERVER_H
#define _TJ_NP_WEBSERVER_H

#include <TJShared/include/tjdispatch.h>
#include "tjnpinternal.h"
#include "tjsocket.h"
#include "tjhttp.h"
#include "tjwebcontent.h"

#pragma warning(push)
#pragma warning(disable:4251 4275)

namespace tj {
	namespace np {
		class WebServer;

		class NP_EXPORTED WebServerThread: public SocketListenerThread, public SocketListener {
			public:
				WebServerThread(tj::shared::ref<WebServer> fs, int port);
				virtual ~WebServerThread();
				virtual void Start();
				virtual void Run();
				virtual void OnReceive(NativeSocket ns);
				virtual unsigned short GetActualPort() const;

			protected:
				tj::shared::weak<WebServer> _fs;
				tj::shared::Event _readyEvent;
				int _port;
				NativeSocket _server4, _server6;
		};


		class NP_EXPORTED WebServerResponseTask: public tj::shared::Task {
			public:
				WebServerResponseTask(NativeSocket ns, tj::shared::ref<WebServer> ws);
				virtual ~WebServerResponseTask();
				virtual void SendError(int code, const tj::shared::String& desc, const tj::shared::String& extraInfo);
				virtual void ServeRequest(tj::shared::ref<HTTPRequest> hrp);
				virtual void Run();

			protected:
				virtual void SendMultiStatusReply(TiXmlDocument& reply);
				virtual void ServeRequestWithResolver(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res);
				virtual void ServeGetRequestWithResolver(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res);
				virtual void ServeOptionsRequestWithResolver(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res);
				virtual void ServePropFindRequestWithResolver(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res);
				virtual void ServePutRequest(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res, const tj::shared::String& restOfPath);
				virtual void ServeMakeCollectionRequest(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res, const tj::shared::String& restOfPath);
				virtual void ServeDeleteRequest(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res, const tj::shared::String& restOfPath);
				virtual void ServeMoveOrCopyRequestWithResolver(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res, const tj::shared::String& restOfPath);
			
			private:
				virtual std::string CreateAllowHeaderFromPermissions(const tj::shared::Flags<WebItem::Permission>& perms);
				
				const static char* KDAVVersion;
				const static char* KServerName;
				NativeSocket _client;
				tj::shared::weak<WebServer> _ws;
				unsigned int _bytesReceived;
				unsigned int _bytesSent;
		};


		class NP_EXPORTED WebServer: public virtual tj::shared::Object {
			friend class WebServerThread;
			friend class WebServerResponseTask;

			public:
				WebServer(unsigned short port, tj::shared::ref<WebItem> defaultResolver = tj::shared::null, unsigned int maxThreads = 5);
				virtual ~WebServer();
				virtual void OnCreated();
				virtual unsigned int GetBytesReceived() const;
				virtual unsigned int GetBytesSent() const;
				virtual unsigned short GetActualPort() const;
				virtual void AddResolver(const tj::shared::String& pathPrefix, tj::shared::strong<WebItem> fr);

				const static unsigned short KPortDontCare = 0;

			protected:
				tj::shared::CriticalSection _lock;
				virtual void AddTask(tj::shared::strong<tj::shared::Task> t);
				unsigned int _bytesReceived;
				unsigned int _bytesSent;

			private:
				std::map< tj::shared::String, tj::shared::ref<WebItem> > _resolvers;
				tj::shared::ref<WebItem> _defaultResolver;
				tj::shared::ref<WebServerThread> _serverThread;
				tj::shared::ref<tj::shared::Dispatcher> _dispatcher;
				unsigned int _maxThreads;
				volatile int _busyThreads;
				unsigned short _port;
				NetworkInitializer _initializer;
		};

	}
}

#pragma warning(pop)
#endif