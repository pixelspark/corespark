#ifndef _TJLOG_H
#define _TJLOG_H

#pragma warning(push)
#pragma warning(disable:4251)

class LogThread;

class EXPORTED Log {
	public:
		static void Write(std::wstring source, std::wstring message);
		static void Show(bool s);
		static void SetWriteToFile(bool f);
	protected:
		static LogThread* _logger;
		static CriticalSection _lock;
		static bool _writeToFile;
};

#pragma warning(pop)

#endif