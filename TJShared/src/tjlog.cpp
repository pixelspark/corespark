#include "../include/tjcore.h"
#include "../include/ui/tjui.h"
using namespace tj::shared;

bool Log::_writeToFile = false;

namespace tj {
	namespace shared {
		class LogEventLogger: public EventLogger {
			public:
				LogEventLogger() {
				}

				virtual ~LogEventLogger() {
				}

				virtual void AddEvent(const std::wstring& message, ExceptionType e, bool read) {
					Log::Write(L"TJShared/LogEventLogger", message);
				}
		};

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
	}
}

ref<EventLogger> Log::_eventLogger;

strong<EventLogger> Log::GetEventLogger() {
	/* This might doubly create a LogEvenLogger, but that's not really bad
	and otherwise, we would need a static CriticalSection / lock here,
	which seems to be causing some trouble at exit */
	if(!_eventLogger) {
		_eventLogger = GC::Hold(new LogEventLogger());
	}
	return _eventLogger;
}

void Log::Write(const std::wstring& source, const std::wstring& message) {
	if(!Zones::LogZone.CanEnter()) {
		return; // cannot log
	}

	std::wostringstream wos;
	wos << source << L' ' << GetCurrentThreadId() << L':' << L' ' << message;
	std::wstring finalMessage = wos.str();
	_logger.Log(finalMessage);

	if(IsDebuggerPresent()) {
		OutputDebugString(finalMessage.c_str());
		OutputDebugString(L"\r\n");
	}
}

void Log::Show(bool t) {
	_logger.Show(t);
}

std::wstring Log::GetContents() {
	ZoneEntry ze(Zones::LogZone);
	return _logger.GetContents();
}

void Log::Stop() {
	PostThreadMessage(_logger.GetID(), WM_QUIT, 0, 0);
	_logger.WaitForCompletion();
}

EventLogger::~EventLogger() {
}
