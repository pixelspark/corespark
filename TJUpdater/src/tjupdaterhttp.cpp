#include "../include/tjupdater.h"
#include <winhttp.h>
using namespace tj::updater;

URL::URL(const std::wstring& url) {
	_uc = new URL_COMPONENTS;
	memset(_uc, 0, sizeof(URL_COMPONENTS));
	_uc->dwStructSize = sizeof(URL_COMPONENTS);
	
	_uc->dwHostNameLength = 254;
	_uc->lpszHostName = new wchar_t[255];

	_uc->dwExtraInfoLength = 1023;
	_uc->lpszExtraInfo = new wchar_t[1024];

	_uc->dwUrlPathLength = 254;
	_uc->lpszUrlPath = new wchar_t[255];

	_ok = (WinHttpCrackUrl(url.c_str(), 0, 0, _uc) == TRUE);
	DWORD er = GetLastError();
	if(er==ERROR_WINHTTP_INTERNAL_ERROR) {
		UpdaterLog::Write(L"Could not crack URL, internal error");
	}
	else if(er==ERROR_WINHTTP_INVALID_URL) {
		UpdaterLog::Write(L"Could not crack URL, URL is invalid");
	}
	else if(er==ERROR_WINHTTP_UNRECOGNIZED_SCHEME) {
		UpdaterLog::Write(L"Could not crack URL, scheme not recognized");
	}
	else if(er==ERROR_NOT_ENOUGH_MEMORY) {
		UpdaterLog::Write(L"Could not crack URL, insufficient memory");
	}
	else if(er==ERROR_INSUFFICIENT_BUFFER) {
		UpdaterLog::Write(L"Could not crack URL, insufficient buffer memory");
	}
}

URL::~URL() {
	delete[] _uc->lpszHostName;
	delete[] _uc->lpszUrlPath;
	delete[] _uc->lpszExtraInfo;
	delete _uc;
}

bool URL::IsSecure() const {
	return IsOK() && (_uc->nScheme==INTERNET_SCHEME_HTTPS);
}

unsigned short URL::GetPort() const {
	return _uc->nPort;
}

bool URL::IsOK() const {
	return _ok;
}

const wchar_t* URL::GetHostName() const {
	if(IsOK()) {
		return _uc->lpszHostName;
	}
	return 0;
}

const wchar_t* URL::GetPath() const {
	if(IsOK()) {
		return _uc->lpszUrlPath;
	}
	return 0;
}

const wchar_t* URL::GetExtra() const {
	if(IsOK()) {
		return _uc->lpszExtraInfo;
	}
	return 0;
}

void CALLBACK HTTPStatusCallback(HINTERNET internet,DWORD_PTR context,DWORD status,LPVOID statusInfo,DWORD statusInfoLength) {
	std::wostringstream wos;
	wos << L"Status= " << status << L" context=" << context << L" ";

	switch(status) {
		case WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:     
			wos << L"WINHTTP_CALLBACK_STATUS_RESOLVING_NAME";
			break;
		case WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:
			wos << L"WINHTTP_CALLBACK_STATUS_NAME_RESOLVED";
			break;
		case WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:  
			wos << L"WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER";
			break;
		case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:    
			wos << L" WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER";
			break;
		case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:        
			wos << L"WINHTTP_CALLBACK_STATUS_SENDING_REQUEST";
			break;
		case WINHTTP_CALLBACK_STATUS_REQUEST_SENT:           
			wos << L"WINHTTP_CALLBACK_STATUS_REQUEST_SENT";
			break;
		case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE:     
			wos << L"WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE";
			break;
		case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:      
			wos << L"WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED";
			break;
		case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:     
			wos << L"WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION";
			break;
		case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:      
			wos << L"WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED";
			break;
		case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:         
			wos << L"WINHTTP_CALLBACK_STATUS_HANDLE_CREATE";
			break;
		case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:         
			wos << L"WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING";
			break;
		case WINHTTP_CALLBACK_STATUS_DETECTING_PROXY:        
			wos << L"WINHTTP_CALLBACK_STATUS_DETECTING_PROXY";
			break;
		case WINHTTP_CALLBACK_STATUS_REDIRECT:               
			wos << L"WINHTTP_CALLBACK_STATUS_REDIRECT";
			break;
		case WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:  
			wos << L"WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE";
			break;
		case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:         
			wos << L"WINHTTP_CALLBACK_STATUS_SECURE_FAILURE";
			break;
		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:      
			wos << L"WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE";
			break;
		case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:         
			wos << L"WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE";
			break;
		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:          
			wos << L"WINHTTP_CALLBACK_STATUS_READ_COMPLETE";
			break;
		case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:         
			wos << L"WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE";
			break;
		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:          
			wos << L"WINHTTP_CALLBACK_STATUS_REQUEST_ERROR";
			break;
		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:   
			wos << L"WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE";
			break;
	}

	if(status==WINHTTP_CALLBACK_STATUS_SECURE_FAILURE) {
		wos << L" flag=" << statusInfo;
	}
	UpdaterLog::Write(wos.str());
}

/** HTTPRequest **/
HTTPRequest::HTTPRequest(const std::wstring &url, const std::wstring& ua, const std::wstring& method): _connected(false) {
	_internet = WinHttpOpen(ua.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if(_internet!=NULL) {
		// Set a logging callback
		WinHttpSetStatusCallback(_internet, HTTPStatusCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, NULL);

		// Parse the URL
		URL urlParsed(url.c_str());
		UpdaterLog::Write(L"HTTP request for URL "+url);

		if(urlParsed.IsOK()) {
			_host = urlParsed.GetHostName();

			INTERNET_PORT port = urlParsed.GetPort();
			bool isSecure = urlParsed.IsSecure();

			std::wostringstream wos;
			wos << L"Using port " << port << L" secure=" << isSecure;
			UpdaterLog::Write(wos.str());

			_connection = WinHttpConnect(_internet, urlParsed.GetHostName(), port, 0);
			if(_connection!=NULL) {
				std::wstring objectName = std::wstring(urlParsed.GetPath()) + urlParsed.GetExtra();
				_request = WinHttpOpenRequest(_connection, method.c_str(), objectName.c_str(), NULL, NULL, NULL, isSecure ? WINHTTP_FLAG_SECURE : 0);

				if(_request!=NULL) {
					// Disable certificate validation if the settings say so
					if(isSecure && UpdaterSettings::GetSetting(L"updater.http.ssl.no-authentication", L"no")==L"yes") {
						UpdaterLog::Write(L"Not using SSL authentication (any server certificate will do)");
						DWORD flags  = SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE; 
						if(!WinHttpSetOption(_request, WINHTTP_OPTION_SECURITY_FLAGS, &flags, sizeof (DWORD))) {
							UpdaterLog::Write(L"Could not set security flags");
						}
					}
					_connected = true;
				}
				else {
					UpdaterLog::Write(L"WinHttpOpenRequest failed");
				}
			}
		}
		else {
			UpdaterLog::Write(L"WinHttpCrackURL failed; URL probably invalid");
		}
	}
	else {
		UpdaterLog::Write(L"WinHttpOpen or WinHttpConnect failed: Could not open dem pipez to zee internet!");
	}	
}

HTTPRequest::~HTTPRequest() {
	WinHttpCloseHandle(_request);
	WinHttpCloseHandle(_connection);
	WinHttpCloseHandle(_internet);
}

bool HTTPRequest::CheckRequestStatusCode() {
	// Check the response code
	DWORD statusCode = 0;
	DWORD statusCodeSize = sizeof(DWORD);

	if(WinHttpQueryHeaders(_request, WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX)==FALSE) {
		UpdaterLog::Write(L"Could not check status response code; assuming an error");
		return false;
	}
	else {
		std::wostringstream wos;
		wos << L"Status code=" << statusCode;
		UpdaterLog::Write(wos.str());

		if(statusCode!=HTTP_STATUS_OK) {
			UpdaterLog::Write(L"Status code is not OK; cancelling request");
			return false;
		}
	}
	return true;
}

bool HTTPRequest::Download(TiXmlDocument& doc) {
	if(_connected) {
		std::wostringstream headers;
		headers << L"Host: " << _host << L"\r\n";
		std::wstring headerString = headers.str();
		if(WinHttpSendRequest(_request, headerString.c_str(), -1L, WINHTTP_NO_REQUEST_DATA, 0, WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, NULL)==TRUE) {
			if(WinHttpReceiveResponse(_request, NULL)==TRUE) {
				if(!CheckRequestStatusCode()) {
					return false;
				}

				std::ostringstream receivedData;
				DWORD size = 0;
				do {
					// Check for available data
					size = 0;
					if(!WinHttpQueryDataAvailable(_request, &size)) {
						UpdaterLog::Write(L"WinHttpQueryDataAvailable failed; no data available");
					}
					else {
						char* buffer = new char[size+1];
						memset(buffer, 0, size*sizeof(char));

						DWORD downloaded = 0;
						if(!WinHttpReadData(_request, (LPVOID)buffer, size, &downloaded)) {
							UpdaterLog::Write(L"WinHttpReadData failed; could not read data");
							delete[] buffer;
							return false;
						}
						else {
							if(downloaded>0) {
								receivedData << buffer;
							}
						}

						delete[] buffer;
					  }
				} while(size > 0);
				
				doc.Parse(receivedData.str().c_str());
			}
			else {
				UpdaterLog::Write(L"WinHttpReceiveResponse failed");
				return false;
			}
		}
		else {
			UpdaterLog::Write(L"WinHttpSendRequest failed");
			return false;
		}
	}
	return true;
}

bool HTTPRequest::Download(const std::wstring& localFile) {
	if(_connected) {
		// Open a handle to the target file
		HANDLE target = CreateFile(localFile.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
		if(target==INVALID_HANDLE_VALUE) {
			UpdaterLog::Write(L"Couldn't open file handle to target file "+localFile);
			return false;
		}

		try {
			std::wostringstream headers;
			headers << L"Host: " << _host << L"\r\n";
			std::wstring headerString = headers.str();
			if(WinHttpSendRequest(_request, headerString.c_str(), -1L, WINHTTP_NO_REQUEST_DATA, 0, WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, NULL)==TRUE) {
				if(WinHttpReceiveResponse(_request, NULL)==TRUE) {
					if(!CheckRequestStatusCode()) {
						return false;
					}

					DWORD size = 0;
					do {
						// Check for available data
						size = 0;
						if(!WinHttpQueryDataAvailable(_request, &size)) {
							throw L"WinHttpQueryDataAvailable failed; no data available";
						}
						else {
							char* buffer = new char[size+1];
							memset(buffer, 0, size*sizeof(char));

							DWORD downloaded = 0;
							if(!WinHttpReadData(_request, (LPVOID)buffer, size, &downloaded)) {
								delete[] buffer;
								throw L"WinHttpReadData failed; could not read data";
							}
							else {
								if(downloaded>0) {
									DWORD written = 0;
									if(WriteFile(target, buffer, downloaded, &written, NULL)==FALSE) {
										delete[] buffer;
										throw L"WriteFile failed";
									}
								}
							}

							delete[] buffer;
						  }
					} while(size > 0);
				}
				else {
					throw L"WinHttpReceiveResponse failed";
					return false;
				}
			}
			else {
				throw L"WinHttpSendRequest failed";
			}
		}
		catch(const wchar_t* x) {
			UpdaterLog::Write(x);
			CloseHandle(target);
			return false;
		}
		CloseHandle(target);
		return true;
	}
	return false;
}

bool HTTPRequest::DownloadToFile(const std::wstring& url, const std::wstring& localFile, const std::wstring& ua) {
	HTTPRequest hrp(url, ua);
	return hrp.Download(localFile);
}