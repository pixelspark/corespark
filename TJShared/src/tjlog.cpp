#include "../include/tjshared.h"

LogThread* Log::_logger = 0;
CriticalSection Log::_lock;

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

		virtual void Log(std::wstring msg) {
			Start();
			WaitForSingleObject(_loggerCreatedEvent, INFINITE);
			_logger->Log(msg);
		}

		virtual void Show(bool s) {
			_logger->Show(s);
		}

	protected:
		virtual void Run() {
			_logger = new LoggerWnd(0L);
			SetEvent(_loggerCreatedEvent);

			MSG msg;
			while(GetMessage(&msg, 0, 0, 0)!=WM_QUIT) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			delete _logger;
		}

		LoggerWnd* _logger;
		HANDLE _loggerCreatedEvent;
};

void Log::Write(std::wstring source, std::wstring message) {
	ThreadLock lock(&_lock);
	if(_logger==0) _logger = new LogThread();
	_logger->Log(source + L": " + message);
}

void Log::Show(bool t) {
	if(_logger==0) return;
	_logger->Show(t);
}