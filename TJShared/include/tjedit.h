#ifndef _TJEDIT_H
#define _TJEDIT_H

class EXPORTED EditWnd: public Wnd {
	public:
		EditWnd(HWND parent);
		virtual ~EditWnd();
		virtual wchar_t GetPreferredHotkey();
		std::wstring GetText();
		void SetCue(std::wstring cue);
		virtual void Paint(Gdiplus::Graphics& g);
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual LRESULT PreMessage(UINT msg, WPARAM wp, LPARAM lp);
	protected:
		HFONT _font;
		WNDPROC _proc;
};

#endif