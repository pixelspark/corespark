#ifndef _TJTAB_H
#define _TJTAB_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED Pane {
	friend class TabWnd;

	public:
		Pane(std::wstring, ref<Wnd>, bool);
		void SetDetached(bool d, TabWnd* tab);
		ref<Wnd> GetWindow();

	protected:
		std::wstring _title;
		ref<Wnd> _wnd;
		bool _detached;
		bool _fullscreen;
};

class EXPORTED TabWnd: public ChildWnd {
	public:
		TabWnd(HWND parent);
		virtual ~TabWnd();
		virtual wchar_t GetPreferredHotkey();
		void SetHotkey(wchar_t hotkey);
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		ref<Pane> AddPane(std::wstring name, ref<Wnd> wnd);
		ref<Pane> AddPane(ref<Pane> pane);
		ref<Pane> GetPane(int index);
		void RemovePane(ref<Wnd> wnd);
		void SelectPane(unsigned int index);
		virtual void Layout();
		virtual void Update();
		virtual void LeaveHotkeyMode(wchar_t key);
		virtual void EnterHotkeyMode();
		virtual bool IsInHotkeyMode();
		virtual void Clear();
		virtual void Paint(Gdiplus::Graphics& g);
		void Rename(ref<Wnd> pane, std::wstring name);
		ref<Wnd> GetCurrentPane();
		ref<Pane> GetPaneAt(int x);
	
	protected:
		void SetDraggingPane(ref<Pane> pane);
		void DoContextMenu(int x, int y);

		wchar_t _hotkey;
		std::vector< ref<Pane> > _panes;
		ref<Pane> _current;
		ref<Pane> _dragging;
		int _headerHeight;

		enum {defaultHeaderHeight = 21};
};

#pragma warning(pop)

#endif