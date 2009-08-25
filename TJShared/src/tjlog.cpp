#include "../include/tjshared.h"
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
	wos << std::hex << std::setw(4) << std::uppercase << std::setfill(L'0') << Thread::GetCurrentThreadID();
	
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
	
	wos << L' ' << source << L' ' << L':' << L' ' << message;
	String finalMessage = wos.str();

	#ifdef TJ_OS_WIN
		if(IsDebuggerPresent()) {
			OutputDebugString(finalMessage.c_str());
			OutputDebugString(L"\r\n");
		}
	#endif

	#ifdef TJ_OS_MAC
		std::wcout << message << std::endl;
	#endif

	GetEventLogger()->AddEvent(finalMessage, ExceptionTypeMessage, false);
}

void Log::SetEventLogger(strong<EventLogger> se) {
	_eventLogger = se;
}

EventLogger::~EventLogger() {
}
