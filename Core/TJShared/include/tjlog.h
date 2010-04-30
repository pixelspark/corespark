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
 
 #ifndef _TJLOG_H
#define _TJLOG_H

#include "tjsharedinternal.h"
#include "tjthread.h"

namespace tj {
	namespace shared {
		class LogThread;

		/** The event logger logs events that matter to the application and to the user. The 'Log' class
		can be used for stuff like debug logging. In TJShow for example, the event log could be transferred
		from client to server, whilst debug logs will probably never be transferred. **/
		class EXPORTED EventLogger {
			public:
				virtual ~EventLogger();
				virtual void AddEvent(const String& message, ExceptionType e, bool read = false) = 0;
		};

		class EXPORTED Log: public virtual Object {
			public:
				static void Write(const String& source, const String& message);
				static strong<EventLogger> GetEventLogger();
				static void SetEventLogger(strong<EventLogger> se);
				static void SetLogToConsole(bool c);
				static void SetLogToSyslog(bool s);

			protected:
				static bool _logToConsole;
				static bool _logToSyslog;
				static ref<EventLogger> _eventLogger;
				static CriticalSection _logLock;
		};
	}
}

#endif
