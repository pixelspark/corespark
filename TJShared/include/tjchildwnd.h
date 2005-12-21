#ifndef _TJCHILDWND_H
#define _TJCHILDWND_H

class EXPORTED ChildWnd: public Wnd {
	public:
		ChildWnd(const wchar_t* title, HWND parent, bool wantDoubleClick=true, bool useDoubleBuffering=true);
		virtual void Fill();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void LeaveHotkeyMode(wchar_t key=0);
};

/* for testing the splitter thing */
class EXPORTED ColorWnd: public ChildWnd {
public:
	ColorWnd(HWND parent,unsigned char r, unsigned char g, unsigned char b): ChildWnd(L"Color", parent) {
		_r = r;
		_g = g;
		_b = b;
	}

	virtual wchar_t GetPreferredHotkey() {
		return L'C';
	}

	virtual ~ColorWnd() {}
	virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);

	unsigned char _r,_g,_b;
};


#endif