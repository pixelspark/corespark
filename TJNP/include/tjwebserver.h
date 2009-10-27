#ifndef _TJ_NP_WEBSERVER_H
#define _TJ_NP_WEBSERVER_H

#include "internal/tjnp.h"
#include "tjsocket.h"

#pragma warning(push)
#pragma warning(disable:4251 4275)

namespace tj {
	namespace np {
		class NP_EXPORTED FileRequest: public tj::shared::Object {
			public:
				virtual ~FileRequest();
				virtual const tj::shared::String& GetParameter(const std::string& parameter, const tj::shared::String& defaultValue) = 0;
				virtual const tj::shared::String& GetPath() const = 0;
		};

		class NP_EXPORTED FileRequestResolver: public virtual tj::shared::Object {
			public:
				enum Resolution {
					ResolutionNone = 0,
					ResolutionNotFound,
					ResolutionFile,
					ResolutionData,
					ResolutionEmpty,
				};

				virtual ~FileRequestResolver();
				virtual Resolution Resolve(tj::shared::ref<FileRequest> frq, tj::shared::String& file, tj::shared::String& error, char** data, unsigned int& dataLength) = 0;
		};

		class NP_EXPORTED HTTPRequest: public FileRequest {
			public:
				enum Method {
					MethodNone = 0,
					MethodGet = 1,
					MethodPost = 2,
				};

				HTTPRequest(const std::string& req);
				virtual ~HTTPRequest();
				virtual const tj::shared::String& GetParameter(const std::string& parameter, const tj::shared::String& defaultValue);
				virtual const tj::shared::String& GetPath() const;
				Method GetMethod() const;
				static char GetHexChar(char a);
				const tj::shared::String& GetQueryString() const;
				static tj::shared::String URLDecode(std::string::const_iterator it, std::string::const_iterator end);

				std::map< std::string, tj::shared::String > _parameters;

			protected:
				enum ParserState {
					ParsingMethod = 0,
					ParsingFile = 1,
					ParsingProtocol,
					ParsingEnd,
				};

				Method _method;
				ParserState _state;
				tj::shared::String _file;
				tj::shared::String _queryString;
		};

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