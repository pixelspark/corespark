#ifndef _TJLOG_H
#define _TJLOG_H

#pragma warning(push)
#pragma warning(disable:4251)

class EXPORTED Log {
	public:
		static void Write(std::wstring source, std::wstring message);
		static void Show(bool s);
	protected:
		static LoggerWnd* _logger;
};

#pragma warning(pop)

#endif