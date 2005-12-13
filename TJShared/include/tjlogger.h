#ifndef _TJLOGGER_H
#define _TJLOGGER_H

class EXPORTED LoggerWnd: public Wnd {
	public:
		LoggerWnd(HWND parent);
		virtual ~LoggerWnd();
		void Log(std::wstring ws);
		wchar_t GetPreferredHotkey();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void Layout();

	protected:
		HFONT _font;
		HWND _list;
};

#endif