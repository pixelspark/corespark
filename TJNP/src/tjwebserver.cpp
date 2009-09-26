#include "../include/tjwebserver.h"
using namespace tj::np;
using namespace tj::shared;

#ifdef TJ_OS_WIN
	#include <winsock2.h>
#endif

/** HTTPRequest **/
HTTPRequest::HTTPRequest(const std::string& req) {
	_method = MethodNone;
	_state = ParsingMethod;

	std::string::const_iterator it = req.begin();
	while(it!=req.end()) {
		// Parsing method
		if(_state==ParsingMethod) {
			std::string::const_iterator end = std::find(it, req.end(), L' ');
			std::string method(it,end);
			if(method=="GET") {
				_method = MethodGet;
				_state = ParsingFile;
			}
			else if(method=="POST") {
				_method = MethodPost;
				_state = ParsingFile;
			}
			it = end+1;
		}
		else if(_state==ParsingFile) {
			std::string::const_iterator end = std::find(it, req.end(), L' ');
			std::string::const_iterator endURI = std::find(it, req.end(), L'?');
			if(endURI!=req.end() && endURI<end) {
				// parameters!
				_file = URLDecode(it,endURI);
				_queryString = URLDecode(endURI+1, end);
				std::string::const_iterator parameterBegin = endURI+1;
				
				while(parameterBegin <= end) {
					std::string::const_iterator endName = std::find(parameterBegin, end, L'=');
					if(endName==end) break;

					std::string parameterName(parameterBegin, endName);
					std::string::const_iterator endValue = std::find(endName, end, L'&');
					if(endValue<=end) {
						_parameters[parameterName] = URLDecode(endName+1, endValue);
						parameterBegin = endValue+1;
					}
					else {
						_parameters[parameterName] = URLDecode(endName+1, end);
						break;
					}
				}
			}
			else {
				_file = URLDecode(it,end);
			}
			it = end+1;
			_state = ParsingProtocol;
		}
		else if(_state==ParsingProtocol) {
			std::string::const_iterator end = std::find(it, req.end(), L'\n');
			it = end+1;
			_state = ParsingEnd;
			break;
		}
	}
}

HTTPRequest::~HTTPRequest() {
}

const std::wstring& HTTPRequest::GetParameter(const std::string& parameter, const std::wstring& defaultValue) {
	std::map< std::string, std::wstring >::const_iterator it = _parameters.find(parameter);
	if(it!=_parameters.end()) {
		return it->second;
	}
	return defaultValue;
}

const std::wstring& HTTPRequest::GetPath() const {
	return _file;
}

HTTPRequest::Method HTTPRequest::GetMethod() const {
	return _method;
}

char HTTPRequest::GetHexChar(char a) {
	char va = 0;

	if(a>='0' && a<='9') {
		va = (a-'0');
	}
	else if(a>='A' && a<='F') {
		va = (a-'A')+10;
	}
	else if(a>='a' && a<='f') {
		va = (a-'a')+10;
	}
	return va;
}

const std::wstring& HTTPRequest::GetQueryString() const {
	return _queryString;
}

std::wstring HTTPRequest::URLDecode(std::string::const_iterator it, std::string::const_iterator end) {
	std::wostringstream os;

	while(it!=end) {
		char current = *it;
		if(current=='+') {
			os << ' ';
		}
		else if(current=='%') {
			++it;
			if(it==end) break;
			char ca = *it;
			if(ca=='%') {
				os << '%';
				++it;
				continue;
			}

			++it;
			if(it==end) break;
			char cb = *it;

			os << (char)(GetHexChar(ca)*16 + GetHexChar(cb));
		}
		else {
			os << current;
		}
		++it;
	}
	return os.str();
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
		reply << ":" << Mbs(extraInfo);
	}

	std::string replyText = reply.str();
	int length = (int)replyText.length();
	send(_client, replyText.c_str(), length, 0);

	ref<WebServer> fs = _fs;
	if(fs) {
		fs->_bytesSent += length;
	}
}

void WebServerResponseThread::ServePage(ref<HTTPRequest> hrp) {
	const std::wstring& requestFile = hrp->GetPath();

	// Check if there is a resolver for the path, otherwise use the default file resolver (this->Resolve).
	// Check if there is a resolver that can resolve this path (by looking at the start of the path)
	ref<FileRequestResolver> resolver;
	ref<WebServer> fs = _fs;
	if(fs) {
		ThreadLock lock(&(fs->_lock));
		resolver = fs->_defaultResolver;

		std::map< std::wstring, ref<FileRequestResolver> >::iterator it = fs->_resolvers.begin();
		while(it!=fs->_resolvers.end()) {
			const std::wstring& resolverPath = it->first;
			if(requestFile.compare(0, resolverPath.length(), resolverPath)==0) {
				// Use this resolver
				resolver = it->second;
				break;
			}
			++it;
		}
	}

	std::wstring resolvedFile;
	std::wstring resolverError;
	char* resolvedData = 0;
	unsigned int resolvedDataLength = 0;
	bool sendData = false;

	if(resolver) {
		FileRequestResolver::Resolution res = resolver->Resolve(hrp, resolvedFile, resolverError, &resolvedData, resolvedDataLength);
		if(res==FileRequestResolver::ResolutionNone) {
			SendError(500, L"Internal server error", resolverError);
			return;
		}
		else if(res==FileRequestResolver::ResolutionNotFound) {
			SendError(404, L"Not found", requestFile);
			return;
		}
		else if(res==FileRequestResolver::ResolutionData) {
			if(resolvedData==0) {
				SendError(500, L"Internal server error", L"No data: "+resolverError);
				return;
			}
			sendData = true;
		}
	}
	else {
		SendError(404, L"Not found", requestFile);
		return;
	}
	
	// Reply
	std::ostringstream headers;
	headers << "HTTP/1.0 200 OK\r\n";
	headers << "Connection: close\r\n";
	headers << "Server: TJNP\r\n";

	// Just dump the file
	if(sendData) {
		int r = send(_client, resolvedData, resolvedDataLength, 0);
		if(r>0) {
			ref<WebServer> fs = _fs;
			if(fs) {
				fs->_bytesSent += r;
			}
		}
		delete[] resolvedData;
	}
	else {
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
	}
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
		if(r==SOCKET_ERROR || r==WSAETIMEDOUT || r==0) {
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
		ServePage(hrp);
	}

	shutdown(_client, SD_BOTH);
	closesocket(_client);
	delete this;
}

WebServerThread::WebServerThread(ref<WebServer> fs, int port): _fs(fs), _port(port) {
}

WebServerThread::~WebServerThread() {
}

void WebServerThread::Run() {
	NativeSocket server = socket(AF_INET, SOCK_STREAM, 0);
	if(server==-1) {
		Log::Write(L"TJNP/WebServer", L"Could not create socket!");
		return;
	}

	sockaddr_in local;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_family = AF_INET;
	local.sin_port = htons(_port);
	
	if(bind(server, (sockaddr*)&local, sizeof(sockaddr_in))!=0) {
		Log::Write(L"TJNP/WebServer", L"Could not bind socket to port (port already taken?)!");
		return;
	}

	if(listen(server, 10)!=0) {
		Log::Write(L"TJNP/WebServer", L"The socket just doesn't want to listen!");
		return;
	}
	
	// TODO: limit the number of threads with some kind of semaphore?
	Log::Write(L"TJNP/WebServer", L"WebServer is up and running");
	while(true) {
		ref<WebServer> fs = _fs;
		if(!fs->_run) {
			break;
		}

		NativeSocket client = accept(server, 0,0);

		if(client!=-1) {
			// start a response thread
			WebServerResponseThread* rt = new WebServerResponseThread(client, _fs); // the thread will delete itself
			rt->Start();
		}
	}

	#ifdef TJ_OS_WIN
		closesocket(server);
	#endif

	#ifdef TJ_OS_POSIX
		close(server);
	#endif

	Log::Write(L"TJNP/WebServer", L"WebServer has been stopped");
}

/** WebServer **/
WebServer::WebServer(unsigned short port, ref<FileRequestResolver> defaultResolver): _run(false), _bytesSent(0), _bytesReceived(0), _defaultResolver(defaultResolver), _port(port) {
}

WebServer::~WebServer() {
	_run = false;
}

void WebServer::OnCreated() {
	_serverThread = GC::Hold(new WebServerThread(this, _port));
	_run = true;
	_serverThread->Start();
}

void WebServer::AddResolver(const std::wstring& pathPrefix, strong<FileRequestResolver> frq) {
	ThreadLock lock(&_lock);
	_resolvers[pathPrefix] = frq;
}

void WebServer::Stop() {
	// TODO implement (stop WebServerThread and all the threads it has spawned)
}

unsigned int WebServer::GetBytesReceived() const {
	return _bytesReceived;
}

unsigned int WebServer::GetBytesSent() const {
	return _bytesSent;
}

/** FileRequestResolver **/
FileRequestResolver::~FileRequestResolver() {
}

/** FileRequest **/
FileRequest::~FileRequest() {
}