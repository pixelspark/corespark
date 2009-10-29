#include "../include/tjhttp.h"
using namespace tj::shared;
using namespace tj::np;

/** Download **/
Download::Download(const NetworkAddress& na, const std::wstring& path, unsigned short port): _path(path), _address(na), _state(DownloadStateNone), _port(port) {
}

Download::~Download() {
}

Download::DownloadState Download::GetState() const {
	return _state;
}

void Download::Start() {
	_state = DownloadStateNone;
	_downloadStateChanged.Reset();
	SocketListenerThread::Start();
}

void Download::OnDownloadComplete(ref<CodeWriter> cw) {
	if(cw) {
		Log::Write(L"TJNP/Download", L"Downloaded "+Stringify(cw->GetSize())+L" bytes");
	}
	else {
		Log::Write(L"TJNP/Download", L"Error occurred, no data");
	}
}

const NetworkAddress& Download::GetAddress() const {
	return _address;
}

void Download::Run() {
	_entersRead = 0;
	_data = GC::Hold(new CodeWriter());
	_socket = GC::Hold(new Socket(_address, TransportProtocolTCP, _port));
	if(_socket->IsValid()) {
		_state = DownloadStateSendingRequest;
		_downloadStateChanged.Signal();
	}
	else {
		_state = DownloadStateCannotConnect;
		_downloadStateChanged.Signal();
		return;
	}
	
	// Compose a nice request
	std::ostringstream request;
	request << "GET " << HTTPRequest::URLEncode(_path) << " HTTP/1.0\r\n\r\n";
	_socket->Send(request.str());
	_state = DownloadStateReceivingHeaders;
	_downloadStateChanged.Signal();
	
	AddListener(_socket->GetNativeSocket(), null);
	SocketListenerThread::Run();
}

void Download::OnReceive(NativeSocket ns) {
	if(ns==_socket->GetNativeSocket()) {
		char buffer[1024];
		unsigned int readBytes = 0;
		
		if(_socket->Read(buffer, 1024, readBytes)) {
			char* dataStart = 0;
			unsigned int dataLength = 0;
			
			// Do not write the header to the file, wait until we recieve \r\n\r\n
			if(_entersRead<4) {
				_state = DownloadStateReceivingHeaders;
				_downloadStateChanged.Signal();
				
				// We need 4 consecutive \r or \n characters to delimit the headers and start the contents
				for(unsigned int a=0;a<readBytes;a++) {
					if(buffer[a]==L'\r' || buffer[a]==L'\n') {
						_entersRead++;
						if(_entersRead>=4) {
							dataStart = &(buffer[a+1]);
							dataLength = readBytes-a-1;
							break;
						}
					}
					else {
						_entersRead = 0;
					}
				}
			}
			// Header was already sent, just dump this packet to the file
			else {
				_state = DownloadStateReceivingData;
				_downloadStateChanged.Signal();
				
				dataStart = buffer;
				dataLength = readBytes;
			}
			
			if(dataStart!=0 && dataLength>0) {
				_data->Append(dataStart, dataLength);
			}
			
			SocketListenerThread::OnReceive(ns);
		}
		else {
			if(!_socket->IsValid()) {
				// Connection was closed
				_state = DownloadStateFinished;
				_downloadStateChanged.Signal();
				OnDownloadComplete(_data);
			}
			else {
				// Error
				_state = DownloadStateError;
				_downloadStateChanged.Signal();
				OnDownloadComplete(null);
			}
		}
	}
}

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

std::string HTTPRequest::URLEncode(const String& rid) {
	std::ostringstream os;
	std::wstring::const_iterator it = rid.begin();
	while(it!=rid.end()) {
		wchar_t current = *it;
		if(current==L' ') {
			os << '+';
		}
		else if(iswascii(current)==0) {
			os << '%' << std::hex << current;
		}
		else {
			os << (char)current;
		}
		++it;
	}
	return os.str();
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

/** FileRequestResolver **/
FileRequestResolver::~FileRequestResolver() {
}

/** FileRequest **/
FileRequest::~FileRequest() {
}
