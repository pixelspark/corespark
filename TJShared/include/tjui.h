#ifndef _TJUI_H
#define _TJUI_H

/** Common GUI classes for usage by both plugins and the program itself **/
#define TJ_DEFAULT_CLASS_NAME (L"TjWndClass")
#define TJ_DEFAULT_NDBL_CLASS_NAME (L"TjWndClassNdbl")
#define TJ_PROPERTY_EDIT_CLASS_NAME (L"TjPropertyEditWndClass")
#define TJ_PROPERTY_EDIT_NUMERIC_CLASS_NAME (L"TjPropertyEditNumericWndClass")
#define  TJ_PROPERTY_LABEL_CLASS_NAME (L"TjPropertyLabelWndClass")

/** Class for initializing GDI+ **/
class EXPORTED GraphicsInit {
	public:
		GraphicsInit();
		virtual ~GraphicsInit();
};

class EXPORTED Wnd {
	public:
		Wnd(const wchar_t* title, HWND parent=0, const wchar_t* className=TJ_DEFAULT_CLASS_NAME);
		virtual ~Wnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void Show(bool s);
		bool IsShown();
		HWND GetWindow();
		void Repaint();
		void SetQuitOnClose(bool q);
		virtual void Layout() {};
		virtual void Update() {};

		virtual void SetText(const wchar_t* t);
		void SetStyle(DWORD style);
		void SetStyleEx(DWORD style);
		void UnsetStyle(DWORD style);
		void UnsetStyleEx(DWORD style);
		void SetHorizontallyScrollable(bool s);
		void SetVerticallyScrollable(bool s);
		unsigned int GetHorizontalPos();
		unsigned int GetVerticalPos();
		void SetVerticalPos(unsigned int p);
		void SetHorizontalPos(unsigned int p);
		Wnd* GetParent();
		virtual bool IsInHotkeyMode(); // returns true if its child windows should show hotkeys

		void SetHorizontalScrollInfo(Range<unsigned int> rng, unsigned int pageSize);
		void SetVerticalScrollInfo(Range<unsigned int> rng, unsigned int pageSize);

		virtual wchar_t GetPreferredHotkey()=0;
		virtual void EnterHotkeyMode();
		virtual void LeaveHotkeyMode(wchar_t key=L'\0');
		virtual void Move(int x, int y, int w, int h);
		virtual bool IsSplitter();

		void SetEatHotkeys(bool e);
		bool IsMouseOver();

	protected:
		void DrawHotkey(Gdiplus::Graphics* g, const wchar_t* wc, int x, int y);
		static void RegisterClasses();
		static bool _classesRegistered;
		HWND _wnd;
		bool _quitOnClose;
		bool _inHotkeyMode;
		bool _eatHotkeys;
		unsigned int _horizontalPos;
		unsigned int _verticalPos;
		unsigned int _horizontalPageSize;
		unsigned int _verticalPageSize;
};

class EXPORTED ChildWnd: public Wnd {
	public:
		ChildWnd(const wchar_t* title, HWND parent, bool wantDoubleClick=true): Wnd(title,parent, wantDoubleClick?TJ_DEFAULT_CLASS_NAME:(L"TjWndClassNdbl")) {
			SetWindowLong(_wnd,GWL_STYLE,WS_CHILD);
			Show(true);
		}

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