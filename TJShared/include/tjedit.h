#ifndef _TJEDIT_H
#define _TJEDIT_H

class EXPORTED EditWnd: public Wnd {
	public:
		EditWnd(HWND parent);
		virtual ~EditWnd();
		virtual wchar_t GetPreferredHotkey();
		std::wstring GetText();
		virtual void Paint(Gdiplus::Graphics& g);

	protected:
		HFONT _font;
};

#endif