#include "../include/tjshared.h"
using namespace tj::shared;

CriticalSection Log::_lock;
bool Log::_writeToFile = false;

namespace tj {
	namespace shared {
		class LogThread: public Thread {
			public:
				LogThread() {
					_loggerCreatedEvent = CreateEvent(NULL, TRUE, FALSE, 0);
					Start();
				}

				virtual ~LogThread() {
					PostThreadMessage(_id, WM_QUIT, 0, 0);
					WaitForCompletion();
				}

				virtual void Log(const std::wstring& msg) {
					//Start();
					WaitForSingleObject(_loggerCreatedEvent, INFINITE);
					_logger->Log(msg);
				}

				virtual void Show(bool s) {
					_logger->Show(s);
				}

				virtual std::wstring GetContents() {
					WaitForSingleObject(_loggerCreatedEvent, INFINITE);
					return _logger->GetContents();
				}

			protected:
				virtual void Run() {
					try {
						_logger = new LoggerWnd();
						SetEvent(_loggerCreatedEvent);

						MSG msg;
						while(GetMessage(&msg, 0, 0, 0)!=WM_QUIT) {
							try {
								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
							catch(Exception& e) {
								MessageBox(0L, e.GetMsg().c_str(), L"Logger Error", MB_OK|MB_ICONERROR);
							}
						}

						delete _logger;
					}
					catch(Exception& e) {
						MessageBox(0L, e.GetMsg().c_str(), L"Logger Error", MB_OK|MB_ICONERROR);
					}
				}

				LoggerWnd* _logger;
				HANDLE _loggerCreatedEvent;
		};

		LogThread Log::_logger;

		class FileLogger {
			protected:
				FileLogger(std::wstring fileName): _file(fileName.c_str()) {
				}

				static ref<FileLogger> _instance;

			public:
				static ref<FileLogger> Instance() {
					if(!_instance) {
						_instance = GC::Hold(new FileLogger(L"tjshow.log"));
					}
					return _instance;
				} 

				virtual ~FileLogger() {
				}

				void Write(const std::wstring& message) {
					_file << message;
					_file.flush();
				}

				std::wofstream _file;
		};
	}
}

ref<FileLogger> FileLogger::_instance;

void Log::Write(const std::wstring& source, const std::wstring& message) {
	ThreadLock lock(&_lock);
	OutputDebugString(message.c_str());
	OutputDebugString(L"\r\n");
	if(_writeToFile) {
		ref<FileLogger> file = FileLogger::Instance();
		file->Write(source + std::wstring(L": ") + message + std::wstring(L"\r\n"));
	}

	std::wstring tid = Stringify(GetCurrentThreadId());

	//if(_logger==0) _logger = new LogThread();
	_logger.Log(source + L" " + tid + L": " + message);
}

void Log::SetWriteToFile(bool f) {
	ThreadLock lock(&_lock);
	_writeToFile = f;
}

void Log::Show(bool t) {
	_logger.Show(t);
}

std::wstring Log::GetContents() {
	ThreadLock lock(&_lock);
	return _logger.GetContents();
}