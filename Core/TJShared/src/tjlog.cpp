/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #include "../include/tjlog.h"
#include "../include/tjthread.h"
#include "../include/tjzone.h"
#include "../include/tjutil.h"
#include <iomanip>

#ifdef TJ_OS_POSIX
	#include <syslog.h>
#endif

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
	bool Log::_logToSyslog = false;
#else
	bool Log::_logToConsole = false;
	bool Log::_logToSyslog = false;
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
	
	if(_logToSyslog) {
		#ifdef TJ_OS_POSIX
			syslog(LOG_INFO, Mbs(wos.str()).c_str());
		#endif
	}

	GetEventLogger()->AddEvent(finalMessage, ExceptionTypeMessage, false);
}

void Log::SetLogToConsole(bool c) {
	_logToConsole = c;
}

void Log::SetLogToSyslog(bool s) {
	#ifdef TJ_OS_POSIX
		ThreadLock lock(&_logLock);
		if(s && !_logToSyslog) {
			openlog("tj",LOG_PID, LOG_USER);
		}
		else if(!s && _logToSyslog) {
			closelog();
		}
	#endif
	
	_logToSyslog = s;
}

void Log::SetEventLogger(strong<EventLogger> se) {
	_eventLogger = se;
}

EventLogger::~EventLogger() {
}
