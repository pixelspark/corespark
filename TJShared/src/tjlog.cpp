#include "../include/tjshared.h"
#include <fstream>


CriticalSection Log::_lock;
bool Log::_writeToFile = false;


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
			//Start();
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

		void Write(std::wstring message) {
			_file << message;
			_file.flush();
		}

		std::wofstream _file;
};

ref<FileLogger> FileLogger::_instance;

void Log::Write(std::wstring source, std::wstring message) {
	ThreadLock lock(&_lock);
	if(_writeToFile) {
		ref<FileLogger> file = FileLogger::Instance();
		file->Write(source + std::wstring(L": ") + message + std::wstring(L"\r\n"));
	}

	//if(_logger==0) _logger = new LogThread();
	_logger.Log(source + L": " + message);
}

void Log::SetWriteToFile(bool f) {
	ThreadLock lock(&_lock);
	_writeToFile = f;
}

void Log::Show(bool t) {
	_logger.Show(t);
}