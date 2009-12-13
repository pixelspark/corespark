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

		class NP_EXPORTED WebServerTask {
			friend class WebServerResponseThread;

			public:
				enum TaskType {
					TaskNone = 0,
					TaskRequest = 1,
					TaskQuit,
				};
			
				WebServerTask(TaskType t);
				TaskType _task;
				NativeSocket _socket;
		};

		class NP_EXPORTED WebServerResponseThread: public tj::shared::Thread {
			public:
				WebServerResponseThread(tj::shared::ref<WebServer> fs);
				virtual ~WebServerResponseThread();
				virtual void SendError(int code, const tj::shared::String& desc, const tj::shared::String& extraInfo);
				virtual void ServeRequest(tj::shared::ref<HTTPRequest> hrp);
				virtual void Run();
				virtual void RunSocket(NativeSocket ns);

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
				WebServer(unsigned short port, tj::shared::ref<WebItem> defaultResolver = tj::shared::null, unsigned int maxThreads = 5);
				virtual ~WebServer();
				virtual void OnCreated();
				virtual unsigned int GetBytesReceived() const;
				virtual unsigned int GetBytesSent() const;
				virtual void Stop();
				virtual unsigned short GetActualPort() const;
				virtual void AddResolver(const tj::shared::String& pathPrefix, tj::shared::strong<WebItem> fr);

				const static unsigned short KPortDontCare = 0;
			
			protected:
				virtual void AddTask(const WebServerTask& wt);

			private:
				tj::shared::CriticalSection _lock;
				std::deque<WebServerTask> _queue;
				tj::shared::Semaphore _queuedTasks;
				std::set< tj::shared::ref<WebServerResponseThread> > _threads;
				std::map< tj::shared::String, tj::shared::ref<WebItem> > _resolvers;
				tj::shared::ref<WebItem> _defaultResolver;
				tj::shared::ref<WebServerThread> _serverThread;
				volatile bool _run;

				unsigned int _bytesReceived;
				unsigned int _bytesSent;
				unsigned int _maxThreads;
				volatile int _busyThreads;
				unsigned short _port;
				NetworkInitializer _initializer;
		};

	}
}

#pragma warning(pop)
#endif