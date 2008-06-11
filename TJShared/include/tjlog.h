#ifndef _TJLOG_H
#define _TJLOG_H

namespace tj {
	namespace shared {
		class LogThread;

		/** The event logger logs events that matter to the application and to the user. The 'Log' class
		can be used for stuff like debug logging. In TJShow for example, the event log could be transferred
		from client to server, whilst debug logs will probably never be transferred. **/
		class EXPORTED EventLogger {
			public:
				virtual ~EventLogger();
				virtual void AddEvent(const std::wstring& message, ExceptionType e, bool read = false) = 0;
		};

		class EXPORTED Log: public virtual Object {
			public:
				static void Write(const std::wstring& source, const std::wstring& message);
				static void Show(bool s);
				static void SetWriteToFile(bool f);
				static std::wstring GetContents();
				static strong<EventLogger> GetEventLogger();
				static void Stop();

			protected:
				static LogThread _logger;
				static CriticalSection _lock;
				static bool _writeToFile;
				static ref<EventLogger> _eventLogger;
		};
	}
}

#endif
