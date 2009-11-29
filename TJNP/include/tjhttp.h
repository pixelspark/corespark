#ifndef _TJ_NP_HTTP_H
#define _TJ_NP_HTTP_H

#include "tjnpinternal.h"
#include "tjsocket.h"
#include "tjwebcontent.h"

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace np {		
		class NP_EXPORTED HTTPRequest: public WebRequest {
			public:
				enum Method {
					MethodNone = 0,
					MethodGet = 1,
					MethodPost = 2,
					MethodOptions = 3,
					MethodPropFind = 4,
					MethodHead = 5,
					MethodDelete = 6,
				};
				
				HTTPRequest(const std::string& req);
				virtual ~HTTPRequest();
				virtual const tj::shared::String& GetParameter(const std::string& parameter, const tj::shared::String& defaultValue);
				virtual const tj::shared::String& GetHeader(const std::string& headerName, const tj::shared::String& defaultValue);
				virtual bool HasHeader(const std::string& headerName) const;
				virtual bool HasParameter(const std::string& paramName) const;
				virtual const tj::shared::String& GetPath() const;
				virtual void SetPath(const tj::shared::String& p);
				Method GetMethod() const;
				static char GetHexChar(char a);
				const tj::shared::String& GetQueryString() const;

				static tj::shared::String URLDecode(std::string::const_iterator it, std::string::const_iterator end);
				static std::string URLEncode(const tj::shared::String& rid);
				static Method MethodFromString(const std::string& methodString);
				
				std::map< std::string, tj::shared::String > _parameters;
				std::map< std::string, tj::shared::String > _headers;
				
			protected:
				enum ParserState {
					ParsingMethod = 0,
					ParsingFile = 1,
					ParsingProtocol,
					ParsingHeaderName,
					ParsingHeaderValue,
					ParsingEnd,
				};
				
				Method _method;
				ParserState _state;
				tj::shared::String _file;
				tj::shared::String _queryString;
		};
		
		class NP_EXPORTED Download: public virtual tj::np::SocketListenerThread {
			enum DownloadState {
				DownloadStateNone = 0,
				DownloadStateConnecting,
				DownloadStateSendingRequest,
				DownloadStateReceivingHeaders,
				DownloadStateReceivingData,
				DownloadStateFinished,
				DownloadStateError,
				DownloadStateCannotConnect,
			};
			
			public:
				Download(const NetworkAddress& na, const tj::shared::String& path, unsigned short port = 80);
				virtual ~Download();
				virtual void Run();
				virtual void Start();
				virtual DownloadState GetState() const;
				virtual const NetworkAddress& GetAddress() const;
				
			protected:
				virtual void OnReceive(NativeSocket ns);
				virtual void OnDownloadComplete(tj::shared::ref<tj::shared::CodeWriter> cw);
				
				std::wstring _path;
				NetworkAddress _address;
				tj::shared::Event _downloadStateChanged;
				volatile DownloadState _state;
				tj::shared::ref<Socket> _socket;
				unsigned short _port;
				tj::shared::ref<tj::shared::CodeWriter> _data;
				unsigned int _entersRead;
		};
	}
}

#pragma warning(pop)
#endif