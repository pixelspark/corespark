#ifndef _TJUI_H
#define _TJUI_H

/** Common GUI classes for usage by both plugins and the program itself **/
#define TJ_DEFAULT_CLASS_NAME (L"TjWndClass")
#define TJ_DEFAULT_NDBL_CLASS_NAME (L"TjWndClassNdbl")
#define TJ_PROPERTY_EDIT_CLASS_NAME (L"TjPropertyEditWndClass")
#define TJ_PROPERTY_EDIT_NUMERIC_CLASS_NAME (L"TjPropertyEditNumericWndClass")
#define TJ_PROPERTY_EDIT_TIME_CLASS_NAME (L"TjPropertyEditTimeWndClass")
#define TJ_PROPERTY_LABEL_CLASS_NAME (L"TjPropertyLabelWndClass")
#define TJ_TAB_PANEL_CLASS_NAME (L"TjTabPanelWndClass")
#define WM_TJ_PRINT (WM_USER+2)
#define ISVKKEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000))

/** Class for initializing GDI+ **/
class EXPORTED GraphicsInit {
	public:
		GraphicsInit();
		virtual ~GraphicsInit();
};

class EXPORTED Wnd: public virtual Object {
	friend class FloatingPane;

	public:
		Wnd(const wchar_t* title, HWND parent=0, const wchar_t* className=TJ_DEFAULT_CLASS_NAME,  bool useDoubleBuffering=true, int exStyle=0L);
		virtual ~Wnd();
		
		virtual void Show(bool s);
		bool IsShown() const;
		HWND GetWindow();
		void Repaint();
		void SetQuitOnClose(bool q);
		virtual void Layout();
		virtual void Update();
		virtual void Paint(Gdiplus::Graphics& g) = 0;
		virtual LRESULT PreMessage(UINT msg, WPARAM wp, LPARAM lp);
		virtual void SetText(const wchar_t* t);
		void SetStyle(DWORD style);
		void SetStyleEx(DWORD style);
		void UnsetStyle(DWORD style);
		void UnsetStyleEx(DWORD style);
		bool IsMouseOver();
		virtual void SetFullScreen(bool f);
		bool IsFullScreen();

		// Scrolling
		void SetHorizontallyScrollable(bool s);
		void SetVerticallyScrollable(bool s);
		unsigned int GetHorizontalPos();
		unsigned int GetVerticalPos();
		void SetVerticalPos(unsigned int p);
		void SetHorizontalPos(unsigned int p);
		void SetHorizontalScrollInfo(Range<unsigned int> rng, unsigned int pageSize);
		void SetVerticalScrollInfo(Range<unsigned int> rng, unsigned int pageSize);

		// Hotkeys
		virtual bool IsInHotkeyMode(); // returns true if its child windows should show hotkeys
		virtual wchar_t GetPreferredHotkey()=0;
		virtual void EnterHotkeyMode();
		virtual void LeaveHotkeyMode(wchar_t key=L'\0');
		virtual void Move(int x, int y, int w, int h);
		virtual bool IsSplitter();
		void SetEatHotkeys(bool e);

		std::wstring GetText();
		void SetText(std::wstring text);
		void SetSize(int w, int h);
		tj::shared::Rectangle GetClientRectangle();
		tj::shared::Rectangle GetWindowRectangle();

		Wnd* GetParent();
		Wnd* GetRootWindow();

		void SetWantMouseLeave(bool t);
		bool GetWantMouseLeave() const;

	protected:
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		void DrawHotkey(Gdiplus::Graphics* g, const wchar_t* wc, int x, int y);
		HWND _wnd;
		bool _inHotkeyMode;
		bool _fullScreen;
		bool _wantsMouseLeave;
	
	private:
		Gdiplus::Bitmap* _buffer;
		bool _doubleBuffered;
		bool _quitOnClose;
		bool _eatHotkeys;
		unsigned int _horizontalPos;
		unsigned int _verticalPos;
		unsigned int _horizontalPageSize;
		unsigned int _verticalPageSize;
		static void RegisterClasses();
		static bool _classesRegistered;
		long _oldStyle, _oldStyleEx;
};

class ChildEnumerator {
	public:
		ChildEnumerator(HWND parent, bool recursive=false);
		void Add(HWND wnd);
	
		std::vector<Wnd*> _children;
		bool _recursive;
		HWND _for;
};


#endif