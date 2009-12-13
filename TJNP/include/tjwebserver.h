#ifndef _TJ_NP_WEBSERVER_H
#define _TJ_NP_WEBSERVER_H

#include "tjnpinternal.h"
#include "tjsocket.h"
#include "tjhttp.h"
#include "tjwebcontent.h"

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
				tj::shared::weak<WebServer> _fs;
		};


		class NP_EXPORTED WebServer: public virtual tj::shared::Object {
			friend class WebServerThread;
			friend class WebServerResponseThread;

			public:
				WebServer(unsigned short port, tj::shared::ref<WebItem> defaultResolver = tj::shared::null);
				virtual ~WebServer();
				virtual void OnCreated();
				virtual unsigned int GetBytesReceived() const;
				virtual unsigned int GetBytesSent() const;
				virtual void Stop();
				virtual unsigned short GetActualPort() const;
				virtual void AddResolver(const tj::shared::String& pathPrefix, tj::shared::strong<WebItem> fr);

				const static unsigned short KPortDontCare = 0;
			
			protected:
				tj::shared::CriticalSection _lock;
				std::map< tj::shared::String, tj::shared::ref<WebItem> > _resolvers;
				tj::shared::ref<WebItem> _defaultResolver;
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