#ifndef _TJUI_H
#define _TJUI_H
#include "../include/tjplatform.h"

/** Class for initializing GDI+ **/
class EXPORTED GraphicsInit {
	public:
		GraphicsInit();
		virtual ~GraphicsInit();
};

class EXPORTED Displays: public virtual Object {
	public:
		Displays();
		virtual ~Displays();
		void AddDisplay(HMONITOR hm);
		RECT GetDisplayRectangle(int idx);
	protected:
		std::vector<HMONITOR> _displays;
};

class EXPORTED Wnd: public virtual Object {
	friend class FloatingPane;

	public:
		Wnd(const wchar_t* title, HWND parent=0, const wchar_t* className=TJ_DEFAULT_CLASS_NAME,  bool useDoubleBuffering=true, int exStyle=0L);
		virtual ~Wnd();
		
		virtual void Show(bool s);
		bool IsShown() const;
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
		virtual void SetFullScreen(bool f, int display);
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

		virtual std::wstring GetText();
		virtual void SetText(std::wstring text);
		void SetSize(int w, int h);
		tj::shared::Rectangle GetClientRectangle();
		tj::shared::Rectangle GetWindowRectangle();

		Wnd* GetParent();
		Wnd* GetRootWindow();

		void SetWantMouseLeave(bool t);
		bool GetWantMouseLeave() const;

		HWND GetWindow();
		virtual std::wstring GetTabTitle() const;

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