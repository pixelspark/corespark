#ifndef _TJLOGGER_H
#define _TJLOGGER_H

class EXPORTED LoggerWnd: public Wnd {
	public:
		LoggerWnd(HWND parent);
		virtual ~LoggerWnd();
		void Log(std::wstring ws);
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void Layout();
		virtual void Paint(Gdiplus::Graphics& g);
		virtual std::wstring GetContents();

	protected:
		HFONT _font;
		HWND _list;
};

#endif