#include "../include/tjshared.h"
#include <iomanip>
using namespace tj::shared;

namespace tj {
	namespace shared {
		class LogEventLogger: public EventLogger {
			public:
				LogEventLogger() {
				}

				virtual ~LogEventLogger() {
				}

				virtual void AddEvent(const String& message, ExceptionType e, bool read) {
				}
		};
	}
}

ref<EventLogger> Log::_eventLogger;
CriticalSection Log::_logLock;

#ifdef TJ_OS_POSIX
	bool Log::_logToConsole = true;
#else
	bool Log::_logToConsole = false;
#endif

strong<EventLogger> Log::GetEventLogger() {
	/* This might doubly create a LogEvenLogger, but that's not really bad
	and otherwise, we would need a static CriticalSection / lock here,
	which seems to be causing some trouble at exit */
	if(!_eventLogger) {
		_eventLogger = GC::Hold(new LogEventLogger());
	}
	return _eventLogger;
}

void Log::Write(const String& source, const String& message) {
	if(!Zones::Get(Zones::LogZone).CanEnter()) {
		return; // cannot log
	}

	std::wostringstream wos;
	wos.width(8);
	wos << std::setw(8) << std::uppercase << std::setfill(L'0') << std::hex << Thread::GetCurrentThreadID();
	
	#ifdef TJ_OS_WIN
		if(Zones::IsDebug() || ::IsDebuggerPresent()) {
			wos << L' ' << Thread::GetCurrentThreadName();
		}
	#endif
	
	#ifdef TJ_OS_MAC
		if(Zones::IsDebug()) {
			wos << L' ' << Thread::GetCurrentThreadName();
		}
	#endif
	
	wos << L' ' << source << L':' << L' ' << message;
	String finalMessage = wos.str();

	#ifdef TJ_OS_WIN
		if(IsDebuggerPresent()) {
			OutputDebugString(finalMessage.c_str());
			OutputDebugString(L"\r\n");
		}
	#endif

	if(_logToConsole) {
		ThreadLock lock(&_logLock);
		std::wcout << std::hex << std::uppercase << std::setw(8) << Thread::GetCurrentThreadID() << L' ' << source << L' ' << L':' << L' ' << message << std::endl;
	}

	GetEventLogger()->AddEvent(finalMessage, ExceptionTypeMessage, false);
}

void Log::SetLogToConsole(bool c) {
	_logToConsole = c;
}

void Log::SetEventLogger(strong<EventLogger> se) {
	_eventLogger = se;
}

EventLogger::~EventLogger() {
}
