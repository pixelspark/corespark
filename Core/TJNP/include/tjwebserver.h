/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
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


		class NP_EXPORTED WebServer: public virtual tj::shared::Object, public SocketListener {
			friend class WebServerResponseTask;

			public:
				WebServer(unsigned short port, tj::shared::ref<WebItem> defaultResolver = tj::shared::null);
				virtual ~WebServer();
				virtual void OnCreated();
				virtual void OnReceive(NativeSocket ns);
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
				tj::shared::ref<tj::shared::Dispatcher> _dispatcher;
				unsigned short _port;
				NativeSocket _server4, _server6;
				tj::shared::ref<SocketListenerThread> _listenerThread;
		};

	}
}

#pragma warning(pop)
#endif