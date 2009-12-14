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
WebServerResponseThread::WebServerResponseThread(ref<WebServer> fs): _fs(fs), _client(-1) {
}

WebServerResponseThread::~WebServerResponseThread() {
	WaitForCompletion();
}

void WebServerResponseThread::SendError(int code, const std::wstring& desc, const std::wstring& extraInfo) {
	std::ostringstream reply;
	reply << "HTTP/1.1 " << code << " Not Found\r\nContent-type: text/html\r\n\r\n <b>" << Mbs(desc) << "</b>";
	if(extraInfo.length()>0) {
		std::wstring extraInfoHTML = extraInfo;
		Util::HTMLEntities(extraInfoHTML);
		reply << ": " << Mbs(extraInfoHTML);
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
	if(perms.IsSet(WebItem::PermissionDelete)) {
		headers << "DELETE, ";
	}
	if(perms.IsSet(WebItem::PermissionPut)) {
		headers << "PUT, ";
	}
	if(perms.IsSet(WebItem::PermissionPropertyRead)) {
		headers << "LOCK, UNLOCK, PROPFIND, ";
	}
	if(perms.IsSet(WebItem::PermissionPropertyWrite)) {
		headers << "MKCOL, MOVE, COPY, ";
	}
	headers << "HEAD, OPTIONS";
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
	else if(res==ResolutionPermissionDenied) {
		SendError(403, L"Forbidden", hrp->GetPath());
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
	bool justHeaders = (hrp->GetMethod()==HTTPRequest::MethodHead);
	std::ostringstream headers;
	headers << "HTTP/1.1 200 OK\r\n";
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
		headers << "Content-Length: " << resolvedDataLength << "\r\n\r\n";
		std::string dataHeaders = headers.str();
		int q = send(_client, dataHeaders.c_str(), dataHeaders.length(), 0);
		int r = 0;
		if(!justHeaders) {
			send(_client, resolvedData, resolvedDataLength, 0);
		}
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
				if(!justHeaders) {
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
			if(!justHeaders) {
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
			}
		#endif
	}
}

void WebServerResponseThread::ServeRequestWithResolver(ref<HTTPRequest> hrp, ref<WebItem> resolver) {
	if(resolver) {
		switch(hrp->GetMethod()) {
			case HTTPRequest::MethodGet:
			case HTTPRequest::MethodHead:
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
				SendError(501, L"Method not implemented", Stringify(hrp->GetMethod()));
		}
	}
	else {
		SendError(404, L"Not found", hrp->GetPath());
		return;
	}
}

void WebServerResponseThread::ServePutRequest(tj::shared::ref<HTTPRequest> hrp, tj::shared::ref<WebItem> res, const tj::shared::String& restOfPath) {
	if(hrp->HasHeader("Content-Range")) {
		SendError(501, L"Not implemented", L"Content-Range header");
		return;
	}

	if(res->Put(restOfPath, hrp->GetAdditionalRequestData())) {
		Log::Write(L"TJNP/WebServer", L"Put OK @"+hrp->GetPath());
		SendError(201, L"Put successful", hrp->GetPath());
	}
	else {
		Log::Write(L"TJNP/WebServer", L"Put failed @"+hrp->GetPath());
		SendError(409, L"Conflict", hrp->GetPath());
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

void WebServerResponseThread::ServeMoveOrCopyRequestWithResolver(ref<HTTPRequest> hrp, ref<WebItem> res, const String& restOfPath) {
	String destination = hrp->GetHeader("Destination", L"");
	if(destination.length()==0) {
		SendError(409, L"Conflict", L"Missing Destination header");
		return;
	}

	// The destination can be an absolute URL; if so, strip it
	if(destination.substr(0, 7)==L"http://") {
		String::size_type idx = destination.find_first_of(L'/', 7);
		if(idx!=String::npos) {
			destination = destination.substr(idx);
		}
	}

	// Strip off the first part of the path that is in the prefix for this resolver
	int prefixLength = hrp->GetPath().length() - restOfPath.length();
	if(prefixLength>0) {
		destination = destination.substr(prefixLength);
	}

	bool overwrite = hrp->GetHeader("Overwrite", L"T")!=L"F";
	if(res->Move(restOfPath, destination, hrp->GetMethod()==HTTPRequest::MethodCopy, overwrite)) {
		SendError(201, L"Created", destination);
	}
	else {
		SendError(409, L"Conflict", L"Operating failed");
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
	if(hrp->HasHeader("Expect")) {
		SendError(417, L"Expectation failed", hrp->GetHeader("Expect",  L""));
		return;
	}

	std::wstring requestFile = hrp->GetPath();

	// If the request URI is absolute; fix it to make it relative
	if(requestFile.substr(0, 7)==L"http://") {
		String::size_type idx = requestFile.find_first_of(L'/', 7);
		if(idx!=String::npos) {
			requestFile = requestFile.substr(idx);
		}
	}

	// The OPTIONS request can have a request URI of '*'; in this case, return the options for '/'
	if(hrp->GetMethod()==HTTPRequest::MethodOptions && requestFile==L"*") {
		requestFile = L"/";
	}

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
					else if(hrp->GetMethod()==HTTPRequest::MethodPut) {
						ServePutRequest(hrp, resolver, restOfPath);
					}
					else if(hrp->GetMethod()==HTTPRequest::MethodCopy || hrp->GetMethod()==HTTPRequest::MethodMove) {
						ServeMoveOrCopyRequestWithResolver(hrp,resolver,restOfPath);
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

void WebServerResponseThread::RunSocket(NativeSocket client) {
	_client = client;

	ref<DataWriter> cwHeaders = GC::Hold(new DataWriter());
	ref<DataWriter> cwData;

	// get request
	char buffer[2048];
	bool readingRequestHeaders = true;
	int64 requestBytesToRead = 0;
	bool readCompleteHeaderBlock = false;
	ref<HTTPRequest> httpRequest;
	int enterCount = 0;

	// TODO: add time-out in select for very slow clients (maybe only time-out when reading headers)
	while(readingRequestHeaders || (requestBytesToRead > 0)) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(_client, &fds);

		if(select(_client+1, &fds, 0, 0, 0)>0) {
			int r = recv(_client, buffer, 2048, 0);
			if(r<=0) {
				readingRequestHeaders = false;
				requestBytesToRead = 0;
				break;
			}
			else {
				ref<WebServer> fs = _fs;
				if(fs) {
					fs->_bytesReceived += r;
				}

				if(readingRequestHeaders) {
					for(int a=0;a<r;a++) {
						if(buffer[a]==L'\r' || buffer[a]==L'\n') {
							enterCount++;
						}
						else {
							enterCount = 0;
						}

						cwHeaders->Append(&(buffer[a]), 1);

						if(enterCount>=4) {
							readCompleteHeaderBlock = true;
							readingRequestHeaders = false;

							// A complete header block was read; let's see if there's additional data
							httpRequest = GC::Hold(new HTTPRequest(cwHeaders,null));
							String contentLength = httpRequest->GetHeader("Content-Length", L"");
							if(contentLength.length()>0) {
								requestBytesToRead = StringTo<int64>(contentLength,0);
								// TODO: limit the maximum number of bytes that can be sent; otherwise, this
								// could crash the server
								if(requestBytesToRead>0) {
									cwData = GC::Hold(new DataWriter(requestBytesToRead));
								}
							}

							// Throw the rest of this block's data in the data buffer
							if(requestBytesToRead>0) {
								int64 dataLeft = r-a-1;
								cwData->Append(&(buffer[a+1]), Util::Min(dataLeft,requestBytesToRead));
								httpRequest->SetAdditionalData(cwData);
								requestBytesToRead -= dataLeft;
							}
						}
					}
				}
				else if(requestBytesToRead>0) {
					cwData->Append(buffer, (unsigned int)Util::Min((int64)r,requestBytesToRead));
					requestBytesToRead -= r;
				}
			}
		}
		else {
			// bail
			break;
		}
	}

	try {
		ServeRequest(httpRequest);
	}
	catch(const Exception& e) {
		Log::Write(L"TJNP/WebServerResponseThread", L"Error occurred when processing request: "+e.GetMsg());
	}
	catch(...) {
		Log::Write(L"TJNP/WebServerResponseThread", L"Unknown error occurred when processing request");
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
}

void WebServerResponseThread::Run() {
	while(true) {
		ref<WebServer> ws = _fs;
		if(!ws) {
			return;
		}
		Semaphore& queueSemaphore = ws->_queuedTasks;
		ws = null;

		// Wait for a task to appear in the queue; if there is a task, wake up and execute it
		if(queueSemaphore.Wait()) {
			WebServerTask task(WebServerTask::TaskNone);
			{
				ws = _fs;
				if(!ws) {
					return;
				}
				ThreadLock lock(&(ws->_lock));	
				std::deque<WebServerTask>::iterator it = ws->_queue.begin();
				if(it==ws->_queue.end()) {
					continue;
				}

				task = *it;
				ws->_queue.pop_front();
				++(ws->_busyThreads);
				ws = null;
			}

			try {
				if(task._task==WebServerTask::TaskRequest) {
					RunSocket(task._socket);
				}
				else if(task._task==WebServerTask::TaskQuit) {
					return;
				}
			}
			catch(const Exception& e) {
				Log::Write(L"TJNP/WebServerResponseThread", L"Error occurred when processing client request: "+e.GetMsg());
			}
			catch(...) {
				Log::Write(L"TJNP/WebServerResponseThread", L"Unknown error occurred when processing client request");
			}

			{
				ws = _fs;
				if(!ws) {
					return;
				}

				ThreadLock lock(&(ws->_lock));	
				--(ws->_busyThreads);
			}
		}
	}
}

WebServerThread::WebServerThread(ref<WebServer> fs, int port): _fs(fs), _port(port), _server4(-1), _server6(-1) {
}

WebServerThread::~WebServerThread() {
	// Termination is handled by SocketListenerThread
}

unsigned short WebServerThread::GetActualPort() const {
	return _port;
}

void WebServerThread::Start() {
	_readyEvent.Reset();
	Thread::Start();
	_readyEvent.Wait(); // Blocks until the web server is initialized, and the actual port is known
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
	
	#ifdef TJ_OS_POSIX
		if(v6) {
			fcntl(_server6, O_NONBLOCK);
		}
		if(v4) {
			fcntl(_server4, O_NONBLOCK);	
		}
	#else
		#warning Need to set non-blocking server socket
	#endif
	
	

	if(v6) {
		AddListener(_server6, this);
	}
	
	if(v4) {
		AddListener(_server4, this);
	}
	
	// TODO: limit the number of threads with some kind of semaphore?
	_readyEvent.Signal();
	if(v4 || v6) {
		Log::Write(L"TJNP/WebServer", L"WebServer is up and running");
	}
	else {
		return;
	}
	
	SocketListenerThread::Run();

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
}

void WebServerThread::OnReceive(NativeSocket ns) {
	NativeSocket client = accept(ns, 0, 0);

	if(client!=-1) {
		// handle request
		ref<WebServer> fs = _fs;
		if(fs) {
			WebServerTask wt(WebServerTask::TaskRequest);
			wt._socket = client;
			fs->AddTask(wt);
		}
		else {
			Stop();
		}
	}
}

/** WebServer **/
WebServer::WebServer(unsigned short port, ref<WebItem> defaultResolver, unsigned int maxThreads): _run(false), _bytesSent(0), _bytesReceived(0), _defaultResolver(defaultResolver), _port(port), _maxThreads(maxThreads), _busyThreads(0) {
}

WebServer::~WebServer() {
	Stop();
}

void WebServer::AddTask(const WebServerTask& wt) {
	ThreadLock lock(&_lock);
	_queue.push_back(wt);
	if(wt._task!=WebServerTask::TaskQuit) {
		int numThreads = _threads.size();
		if((numThreads<1) || ((_busyThreads>=numThreads) && (int(_maxThreads)>numThreads))) {
			// Create a new response thread
			ref<WebServerResponseThread> wrt = GC::Hold(new WebServerResponseThread(this));
			_threads.insert(wrt);
			wrt->Start();
		}
	}
	_queuedTasks.Release();
}

void WebServer::OnCreated() {
	_serverThread = GC::Hold(new WebServerThread(this, _port));
	_run = true;
	_serverThread->Start();
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
	_run = false;
	_serverThread->Stop();

	// Send quit messages
	{
		ThreadLock lock(&_lock);
		std::set< ref<WebServerResponseThread> >::iterator tit = _threads.begin();
		while(tit!=_threads.end()) {
			AddTask(WebServerTask(WebServerTask::TaskQuit));
			++tit;
		}
	}

	_threads.clear(); // ~WebServerResponseThread will wait for completion
	Log::Write(L"TJNP/WebServer", L"Web server stopped");
}

unsigned int WebServer::GetBytesReceived() const {
	return _bytesReceived;
}

unsigned int WebServer::GetBytesSent() const {
	return _bytesSent;
}

/** WebServerTask **/
WebServerTask::WebServerTask(TaskType t): _task(t) {
}
