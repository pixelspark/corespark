#include "../include/tjwebserver.h"
using namespace tj::np;
using namespace tj::shared;

#include <algorithm>

#ifdef TJ_OS_WIN
	#include <winsock2.h>
#endif

#ifdef TJ_OS_POSIX
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
#endif

#ifdef TJ_OS_LINUX
	#include <sys/sendfile.h>
#endif

const char* WebServerResponseThread::KDAVVersion = "1";
const char* WebServerResponseThread::KServerName = "TJNP";

std::ostream& operator<< (std::ostream& out, const TiXmlNode& doc) {
	TiXmlPrinter printer;
	doc.Accept(&printer);
	out << printer.Str();
	return out;
}

/** WebServerResponseThread **/
WebServerResponseThread::WebServerResponseThread(NativeSocket client, ref<WebServer> fs): _fs(fs), _client(client) {
}

WebServerResponseThread::~WebServerResponseThread() {
}

void WebServerResponseThread::SendError(int code, const std::wstring& desc, const std::wstring& extraInfo) {
	std::ostringstream reply;
	reply << "HTTP/1.1 " << code << " Not Found\r\nContent-type: text/html\r\n\r\n<b>" << Mbs(desc) << "</b>";
	if(extraInfo.length()>0) {
		std::wstring extraInfoHTML = extraInfo;
		Util::HTMLEntities(extraInfoHTML);
		reply << ":" << Mbs(extraInfoHTML);
	}

	std::string replyText = reply.str();
	int length = (int)replyText.length();
	send(_client, replyText.c_str(), length, 0);

	ref<WebServer> fs = _fs;
	if(fs) {
		fs->_bytesSent += length;
	}
}


void WebServerResponseThread::SendMultiStatusReply(TiXmlDocument& reply) {	
	std::ostringstream xos;
	xos << reply;	
	std::string dataString = xos.str();

	std::ostringstream os;
	os << "HTTP/1.1 207 Multi-Status\r\n";
	os << "Content-type: text/xml; charset=\"utf-8\"\r\n";
	os << "Content-length: " << int(dataString.length()) << "\r\n";
	os << "Server: " << KServerName << "\r\n";
	os << "Connection: close\r\n";
	os << "DAV: " << KDAVVersion << "\r\n";
	os << "\r\n";
	os << dataString;
	std::string responseString = os.str();

	int q = send(_client, responseString.c_str(), responseString.length(), 0);
	ref<WebServer> fs = _fs;
	if(fs && q>0) {
		fs->_bytesSent += q;
	}
}

class PropFindItemWalker: public WebItemWalker {
	public:
		// host is in the form http://somehost:1234 
		PropFindItemWalker(TiXmlElement* multistatus, const std::string& host): _elm(multistatus), _host(host) {
			assert(multistatus!=0);
		}

		virtual ~PropFindItemWalker() {
		}

		virtual void Add(const tj::shared::String& prefix, ref<WebItem> wi, int level) {
			String myPrefix = prefix;
			if(prefix.at(0)==L'/') {
				myPrefix = prefix.substr(1);
			}
			if((*myPrefix.rbegin())==L'/') {
				myPrefix = myPrefix.substr(0,myPrefix.length()-1);
			}

			TiXmlElement response("response");
			SaveAttribute(&response, "href", _host+"/"+Mbs(myPrefix));
		
			TiXmlElement propstat("propstat");
			SaveAttribute(&propstat, "status", std::wstring(L"HTTP/1.1 200 OK"));

			TiXmlElement prop("prop");
			SaveAttribute(&prop, "displayname", wi->GetDisplayName());
			unsigned int contentLength = wi->GetContentLength();
			if(contentLength>0) {
				SaveAttribute(&prop, "getcontentlength", contentLength);
			}
			SaveAttribute(&prop, "getcontenttype", wi->GetContentType());
			SaveAttribute(&prop, "getetag", wi->GetETag());

			// TODO: create WebItem::GetModifiedDate and support it
			// The date format is "Mon, 04 Apr 2005 23:45:56 GMT".
			SaveAttribute(&prop, "getlastmodified", std::wstring(L""));

			TiXmlElement resourceType("resourcetype");
			if(wi->IsCollection()) {
				TiXmlElement typecollection("collection");
				resourceType.InsertEndChild(typecollection);
			}
			else {
				TiXmlElement typeresource("resource");
				resourceType.InsertEndChild(typeresource);
			}
			prop.InsertEndChild(resourceType);
			propstat.InsertEndChild(prop);

			response.InsertEndChild(propstat);
			_elm->InsertEndChild(response);

			wi->Walk(ref<WebItemWalker>(this), myPrefix, level);
		}

	protected:
		TiXmlElement* _elm;
		std::string _host;
};

void WebServerResponseThread::ServePropFindRequestWithResolver(ref<HTTPRequest> hrp, ref<WebItem> resolver) {
	if(resolver) {
		Flags<WebItem::Permission> perms = resolver->GetPermissions();
		if(perms.IsSet(WebItem::PermissionPropertyRead)) {
			int depth = -1;
			if(hrp->HasHeader("Depth")) {
				std::wstring depthString = hrp->GetHeader("Depth", L"1");
				if(depthString!=L"Infinity") {
					depth = StringTo<int>(depthString, 1);
				}
			}

			TiXmlDocument doc;
			TiXmlDeclaration decl("1.0", "", "no");
			doc.InsertEndChild(decl);

			TiXmlElement multiStatus("multistatus");
			SaveAttributeSmall(&multiStatus, "xmlns", std::wstring(L"DAV:"));
			
			std::string host = "http://" + Mbs(hrp->GetHeader("Host", L"localhost"));
			ref<PropFindItemWalker> pfi = GC::Hold(new PropFindItemWalker(&multiStatus, host));
			pfi->Add(hrp->GetPath(), resolver, depth);

			doc.InsertEndChild(multiStatus);
			SendMultiStatusReply(doc);
		}
		else {
			SendError(405, L"PROPFIND Method not allowed", hrp->GetPath());
		}
	}
	else {
		SendError(404, L"Not found", hrp->GetPath());
	}
}

std::string WebServerResponseThread::CreateAllowHeaderFromPermissions(const Flags<WebItem::Permission>& perms) {
	std::ostringstream headers;
	headers << "Allow: ";
	if(perms.IsSet(WebItem::PermissionGet)) {
		headers << "GET, ";
	}
	if(perms.IsSet(WebItem::PermissionPropertyRead)) {
		headers << "LOCK, UNLOCK, PROPFIND, ";
	}
	if(perms.IsSet(WebItem::PermissionPropertyWrite)) {
		headers << "MKCOL, ";
	}
	headers << "OPTIONS";
	return headers.str();
}

void WebServerResponseThread::ServeOptionsRequestWithResolver(ref<HTTPRequest> hrp, ref<WebItem> resolver) {
	if(resolver) {
		std::ostringstream headers;
		headers << "HTTP/1.1 200 OK\r\n";
		headers << "Connection: close\r\n";
		headers << "Server: " << KServerName << "\r\n";

		Flags<WebItem::Permission> perms = resolver->GetPermissions();
		if(perms.IsSet(WebItem::PermissionPropertyRead)) {
			headers << "DAV: " << KDAVVersion << "\r\n";
		}
		headers << CreateAllowHeaderFromPermissions(perms) << "\r\n";
		
		headers << "\r\n";

		std::string headerString = headers.str();
		int q = send(_client, headerString.c_str(), headerString.length(), 0);
		ref<WebServer> fs = _fs;
		if(fs) {
			fs->_bytesSent += q;
		}
	}
	else {
		SendError(404, L"Not found", hrp->GetPath());
	}
}

void WebServerResponseThread::ServeGetRequestWithResolver(ref<HTTPRequest> hrp, ref<WebItem> resolver) {
	std::wstring resolverError;
	char* resolvedData = 0;
	unsigned int resolvedDataLength = 0;
	bool sendData = false;

	Flags<WebItem::Permission> perms = resolver->GetPermissions();
	if(!perms.IsSet(WebItem::PermissionGet)) {
		SendError(403, L"Permission denied", hrp->GetPath());
		return;
	}

	Resolution res = resolver->Get(hrp, resolverError, &resolvedData, resolvedDataLength);
	if(res==ResolutionNone) {
		SendError(500, L"Internal server error", resolverError);
		return;
	}
	else if(res==ResolutionNotFound) {
		SendError(404, L"Not found", hrp->GetPath());
		return;
	}
	else if(res==ResolutionData) {
		if(resolvedData==0) {
			SendError(500, L"Internal server error", L"No data: "+resolverError);
			return;
		}
		sendData = true;
	}
	else if(res==ResolutionEmpty) {
		SendError(200, L"OK", hrp->GetPath());
		return;
	}

	// Reply
	std::ostringstream headers;
	headers << "HTTP/1.0 200 OK\r\n";
	headers << "Connection: close\r\n";
	headers << "Server: " << KServerName << "\r\n";

	std::string contentType = Mbs(resolver->GetContentType());
	if(contentType.length()>0) {
		headers << "Content-type: " << contentType << "\r\n";
	}

	unsigned int contentLength = resolver->GetContentLength();
	if(contentLength>0) {
		headers << "Content-length: " << int(contentLength) << "\r\n";
	}

	if(perms.IsSet(WebItem::PermissionPropertyRead)) {
		headers << "DAV: " << KDAVVersion << "\r\n";
	}
	headers << CreateAllowHeaderFromPermissions(perms) << "\r\n";

	// Just dump the file
	if(sendData) {
		headers << "Content-length: " << resolvedDataLength << "\r\n\r\n";
		std::string dataHeaders = headers.str();
		int q = send(_client, dataHeaders.c_str(), dataHeaders.length(), 0);
		int r = send(_client, resolvedData, resolvedDataLength, 0);
		if((q+r)>0) {
			ref<WebServer> fs = _fs;
			if(fs) {
				fs->_bytesSent += q+r;
			}
		}
		delete[] resolvedData;
	}
	else {
		std::wstring resolvedFile = resolverError;

		#ifdef TJ_OS_WIN
			HANDLE file = CreateFile(resolvedFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
			DWORD size = GetFileSize(file, 0);
			if(size==INVALID_FILE_SIZE) {
				Log::Write(L"TJNP/WebServer", L"Invalid file size for "+resolvedFile);
			}
			else {
				// Write headers
				headers << "Content-length: " << size << "\r\n\r\n";
				std::string headerString = headers.str();
				int length = (int)headerString.length();
				send(_client, headerString.c_str(), length, 0);

				ref<WebServer> fs = _fs;
				if(fs) {
					fs->_bytesSent += length;
				}

				// Send file
				char buffer[4096];
				DWORD read = 0;

				while(ReadFile(file, buffer, 4096, &read, NULL)!=0) {
					if(read<=0) {
						break;
					}
					int r = send(_client, buffer, read, 0);
					if(r>0) {
						if(fs) {
							fs->_bytesSent += r;
						}
					}
				}
			}

			CloseHandle(file);
		#endif
		
		#ifdef TJ_OS_POSIX
			Bytes size = File::GetFileSize(resolvedFile.c_str());
	
			// Write headers
			headers << "Content-length: " << size << "\r\n\r\n";
			std::string headerString = headers.str();
			int length = (int)headerString.length();
			send(_client, headerString.c_str(), length, 0);
			
			ref<WebServer> fs = _fs;
			if(fs) {
				fs->_bytesSent += length;
			}
	
			// Write data
			int fp = open(Mbs(resolvedFile).c_str(), O_RDONLY);
			if(fp!=-1) {
				off_t length = size;
				off_t start = 0;
				
				#ifdef TJ_OS_MAC
					if(sendfile(fp, _client, start, &length, NULL, 0)!=0) {
						Log::Write(L"TJNP/WebServer", L"sendfile() failed, file path was "+resolvedFile);
					}
				#endif
				
				#ifdef TJ_OS_LINUX
					if(sendfile(_client, fp, NULL, length)==-1) {
						Log::Write(L"TJNP/WebServer", L"sendfile() failed, file path was "+resolvedFile);
					}
				#endif
					
			}
			else {
				Log::Write(L"TJNP/WebServer", L"open() failed, file path was "+resolvedFile);
			}
			close(fp);
		#endif
	}
}

void WebServerResponseThread::ServeRequestWithResolver(ref<HTTPRequest> hrp, ref<WebItem> resolver) {
	if(resolver) {
		switch(hrp->GetMethod()) {
			case HTTPRequest::MethodGet:
				ServeGetRequestWithResolver(hrp, resolver);
				break;

			case HTTPRequest::MethodOptions:
				ServeOptionsRequestWithResolver(hrp,resolver);
				break;

			case HTTPRequest::MethodPropFind:
				ServePropFindRequestWithResolver(hrp,resolver);
				break;

			// Fake lock implementation
			case HTTPRequest::MethodLock:
				SendError(412, L"Precondition Failed", Stringify(hrp->GetPath()));
				break;

			case HTTPRequest::MethodUnlock:
				SendError(204, L"No content", Stringify(hrp->GetPath()));

			default:
				SendError(403, L"Method not implemented", Stringify(hrp->GetMethod()));
		}
	}
	else {
		SendError(404, L"Not found", hrp->GetPath());
		return;
	}
}

void WebServerResponseThread::ServeDeleteRequest(ref<HTTPRequest> hrp, ref<WebItem> item, const String& restOfPath) {
	if(item->Delete(restOfPath)) {
		SendError(204, L"Delete successful", hrp->GetPath());
	}
	else {
		SendError(404, L"Delete not successful", hrp->GetPath());
	}
}

void WebServerResponseThread::ServeMakeCollectionRequest(ref<HTTPRequest> hrp, ref<WebItem> item, const String& restOfPath) {
	ref<WebItem> coll = item->CreateCollection(restOfPath);
	if(coll) {
		SendError(201, L"Collection created", hrp->GetPath());
	}
	else {
		// TODO: be more specific; send 403 Forbidden if it is a permission problem
		SendError(409, L"Conflict", hrp->GetPath());
	}
}

void WebServerResponseThread::ServeRequest(ref<HTTPRequest> hrp) {
	const std::wstring& requestFile = hrp->GetPath();

	// Check if there is a resolver for the path, otherwise use the default file resolver (this->Resolve).
	// Check if there is a resolver that can resolve this path (by looking at the start of the path)
	ref<WebItem> resolver;
	ref<WebServer> fs = _fs;
	if(fs) {
		ThreadLock lock(&(fs->_lock));
		resolver = fs->_defaultResolver;

		std::map< std::wstring, ref<WebItem> >::iterator it = fs->_resolvers.begin();
		while(it!=fs->_resolvers.end()) {
			const std::wstring& resolverPath = it->first;
			if(requestFile.compare(0, resolverPath.length(), resolverPath)==0) {
				// Use this resolver
				resolver = it->second;
				std::wstring restOfPath = requestFile.substr(resolverPath.length());
				
				if(resolver) {
					// For all request that have a path that does not exist right now, don't resolve,
					// and let the request handler fix the problem for us.
					if(hrp->GetMethod()==HTTPRequest::MethodMakeCollection) {
						ServeMakeCollectionRequest(hrp, resolver, restOfPath);
					}
					else if(hrp->GetMethod()==HTTPRequest::MethodDelete) {
						ServeDeleteRequest(hrp, resolver, restOfPath);
					}
					else {
						resolver = resolver->Resolve(restOfPath);
					}
				}
				break;
			}
			++it;
		}
	}

	ServeRequestWithResolver(hrp, resolver);
}

void WebServerResponseThread::Run() {
	std::ostringstream rid;

	// get request
	char buffer[1024];
	bool readingRequest = true;
	int enterCount = 0;

	// TODO: time-out (use select()?)
	while(readingRequest) {
		int r = recv(_client, buffer, 1023, 0);
		if(r<=0) {
			break;
		}
		else {
			ref<WebServer> fs = _fs;
			if(fs) {
				fs->_bytesReceived += r;
			}

			for(int a=0;a<r;a++) {
				if(buffer[a]==L'\0') {
					readingRequest = false;
					break; // end of request 
				}
				else if(buffer[a]==L'\r' || buffer[a]==L'\n') {
					enterCount++;
				}
				else {
					enterCount = 0;
				}

				if(enterCount>=4) {
					readingRequest = false; // for HTTP
					break;
				}
				rid << buffer[a];
			}
		}
	}

	// Check if a complete request was read
	if(!readingRequest) {
		std::string request = rid.str();
		ref<HTTPRequest> hrp = GC::Hold(new HTTPRequest(request));
		ServeRequest(hrp);
	}

	#ifdef TJ_OS_POSIX
		shutdown(_client, SHUT_WR);
	#endif

	#ifdef TJ_OS_WIN
		shutdown(_client, SD_SEND);
	#endif

	while(true) {
		int r = recv(_client, buffer, 1023, 0);
		if(r<=0) {
			// Socket error or graceful close
			break;
		}
	}

	#ifdef TJ_OS_WIN
		closesocket(_client);
	#endif
	
	#ifdef TJ_OS_POSIX
		close(_client);
	#endif
	delete this;
}

WebServerThread::WebServerThread(ref<WebServer> fs, int port): _fs(fs), _port(port), _server4(-1), _server6(-1) {
	#ifdef TJ_OS_POSIX
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, _controlSocket)!=0) {
			Log::Write(L"TJNP/SocketListenerThread", L"Could not create control socket pair");
		}
	#endif
}

WebServerThread::~WebServerThread() {
	Cancel();
	WaitForCompletion();

	#ifdef TJ_OS_POSIX
		close(_controlSocket[0]);
		close(_controlSocket[1]);	
	#endif
}

unsigned short WebServerThread::GetActualPort() const {
	return _port;
}

void WebServerThread::Start() {
	_readyEvent.Reset();
	Thread::Start();
	_readyEvent.Wait(); // Blocks until the web server is initialized, and the actual port is known
}

void WebServerThread::Cancel() {
	#ifdef TJ_OS_POSIX
		char quit[1] = {'Q'};
		if(write(_controlSocket[0], quit, 1)==-1) {
			Log::Write(L"TJNP/SocketListenerThread", L"Could not send quit message to listener thread");
		}
	#endif
	
	#ifdef TJ_OS_WIN
		shutdown (_server4, SD_RECEIVE);
		shutdown (_server6, SD_RECEIVE);
	#endif
}

void WebServerThread::Run() {
	NetworkInitializer ni;
	bool v6 = true;
	bool v4 = true;

	_server6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if(_server6==-1) {
		Log::Write(L"TJNP/WebServer", L"Could not create IPv6 server socket!");
		v6 = false;
	}
	
	_server4 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(_server4==-1) {
		Log::Write(L"TJNP/WebServer", L"Could not create IPv4 server socket!");
		v4 = false;
	}

	in6_addr any = IN6ADDR_ANY_INIT;
	sockaddr_in6 local;
	local.sin6_addr = any;
	local.sin6_family = AF_INET6;
	local.sin6_port = htons(_port);
	
	sockaddr_in local4;
	memset(&local4, 0, sizeof(local4));
	local4.sin_family = AF_INET;
	local4.sin_addr.s_addr = INADDR_ANY;
	local4.sin_port = htons(_port);
	
	if(v4 && bind(_server4, (sockaddr*)&local4, sizeof(sockaddr_in))!=0) {
		Log::Write(L"TJNP/WebServer", L"Could not bind IPv4 socket to port (port already taken?)!");
		v4 = false;
	}

	// If the IPv4 server initialized correctly and chose a port (local4.sin_port==0), then try to find out
	// on which port it is and register the same port for the IPv6 server.
	socklen_t len = sizeof(sockaddr_in);
	if(v4 && v6 && getsockname(_server4, (sockaddr*)&local4, &len)==0) {
		local.sin6_port = local4.sin_port;
	}

	if(v6 && bind(_server6, (sockaddr*)&local, sizeof(sockaddr_in6))!=0) {
		Log::Write(L"TJNP/WebServer", L"Could not bind IPv6 socket to port (port already taken?)!");
		v6 = false;
	}
	
	if(_port==WebServer::KPortDontCare) {
		// Try to find out on which port we are anyway
		if(v4 && getsockname(_server4, (sockaddr*)&local4, &len)==0) {
			_port = ntohs(local4.sin_port);
			Log::Write(L"TNP/WebServer", L"IPv4 web server chose port number: "+Stringify(_port));
		}
	   len = sizeof(sockaddr_in6);
		if(v6 && getsockname(_server6, (sockaddr*)&local, &len)==0) {
			_port = ntohs(local.sin6_port);
			Log::Write(L"TNP/WebServer", L"IPv6 web server chose port number: "+Stringify(_port));
		}  
	}

	if(!v6 || listen(_server6, 10)!=0) {
		Log::Write(L"TJNP/WebServer", L"The IPv6 socket just doesn't want to listen!");
		v6 = false;
	}
	
	if(!v4 || listen(_server4, 10)!=0) {
		Log::Write(L"TJNP/WebServer", L"The IPv4 socket just doesn't want to listen!");
		v4 = false;
	}
	
	// TODO: limit the number of threads with some kind of semaphore?
	_readyEvent.Signal();
	if(v4 || v6) {
		Log::Write(L"TJNP/WebServer", L"WebServer is up and running");
	}
	else {
		return;
	}
	
	
	while(true) {
		ref<WebServer> fs = _fs;
		if(!fs->_run) {
			break;
		}
		
		fd_set fds;
		FD_ZERO(&fds);
		int maxSocket = 0;
		
		#ifdef TJ_OS_POSIX
			FD_SET(_controlSocket[1], &fds); maxSocket = Util::Max(maxSocket, _controlSocket[1]);
		#endif
		
		if(v6) {
			FD_SET(_server6, &fds); 
			maxSocket = Util::Max(maxSocket, (int)_server6);
		}
		
		if(v4) {
			FD_SET(_server4, &fds); 
			maxSocket = Util::Max(maxSocket, (int)_server4);
		}
		
		if(select(maxSocket+1, &fds, NULL, NULL, NULL)>0) {
			#ifdef TJ_OS_POSIX
				if(FD_ISSET(_controlSocket[1], &fds)) {
					// End the thread, a control message was sent to us
					Log::Write(L"TJNP/WebServerThread", L"End thread, quit control message received");
					return;
				}
			#endif
			
			if(FD_ISSET(_server6, &fds)) {
				NativeSocket client = accept(_server6, 0, 0);

				if(client!=-1) {
					// start a response thread
					WebServerResponseThread* rt = new WebServerResponseThread(client, _fs); // the thread will delete itself
					rt->Start();
				}
			}
			else if(FD_ISSET(_server4, &fds)) {
				NativeSocket client = accept(_server4, 0, 0);
				
				if(client!=-1) {
					// start a response thread
					WebServerResponseThread* rt = new WebServerResponseThread(client, _fs); // the thread will delete itself
					rt->Start();
				}
			}
		}
		else {
			Log::Write(L"TJNP/WebServerThread", L"Select operation failed; terminating thread!");
			break;
		}
	}

	#ifdef TJ_OS_WIN
		closesocket(_server6);
		closesocket(_server4);
	#endif

	#ifdef TJ_OS_POSIX
		close(_server6);
		close(_server4);
	#endif

	_server6 = -1;
	_server4 = -1;

	Log::Write(L"TJNP/WebServer", L"WebServer has been stopped");
}

/** WebServer **/
WebServer::WebServer(unsigned short port, ref<WebItem> defaultResolver): _run(false), _bytesSent(0), _bytesReceived(0), _defaultResolver(defaultResolver), _port(port) {
}

WebServer::~WebServer() {
	Stop();
}

void WebServer::OnCreated() {
	_serverThread = GC::Hold(new WebServerThread(this, _port));
	_run = true;
	_serverThread->Start();
	//Log::Write(L"TJNP/WebServer", L"Actual port is "+Stringify(_serverThread->GetActualPort()));
}

void WebServer::AddResolver(const std::wstring& pathPrefix, strong<WebItem> frq) {
	ThreadLock lock(&_lock);
	_resolvers[pathPrefix] = frq;
}

unsigned short WebServer::GetActualPort() const {
	if(_serverThread) {
		// WebServerThread::_port is initialized to KPortDontCare when it is not specified by WebServer
		return _serverThread->GetActualPort();
	}
	return KPortDontCare;
}

void WebServer::Stop() {
	// TODO implement (stop WebServerThread and all the threads it has spawned)
	_run = false;
	_serverThread->Cancel();
}

unsigned int WebServer::GetBytesReceived() const {
	return _bytesReceived;
}

unsigned int WebServer::GetBytesSent() const {
	return _bytesSent;
}