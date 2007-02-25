#ifndef _TJLOG_H
#define _TJLOG_H

namespace tj {
	namespace shared {
		class LogThread;

		class EXPORTED Log: public virtual Object {
			public:
				static void Write(std::wstring source, std::wstring message);
				static void Show(bool s);
				static void SetWriteToFile(bool f);
				static std::wstring GetContents();
			protected:
				static LogThread _logger;
				static CriticalSection _lock;
				static bool _writeToFile;
		};
	}
}

#endif