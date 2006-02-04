#ifndef _TJTAB_H
#define _TJTAB_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED TabWnd: public ChildWnd {
	public:
		TabWnd(HWND parent);
		virtual ~TabWnd();
		virtual wchar_t GetPreferredHotkey();
		void SetHotkey(wchar_t hotkey);
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		void AddPane(std::wstring name, ref<Wnd> wnd);
		void RemovePane(ref<Wnd> wnd);
		void SelectPane(unsigned int index);
		virtual void Layout();
		virtual void Update();
		virtual void LeaveHotkeyMode(wchar_t key);
		virtual void EnterHotkeyMode();
		virtual bool IsInHotkeyMode();
		virtual void Clear();
		virtual void Paint(Gdiplus::Graphics& g);
		ref<Wnd> GetCurrentPane();
	
	protected:
		struct EXPORTED Pane {
			Pane(std::wstring, ref<Wnd>, bool);
			void SetDetached(bool d, TabWnd* tab);

			std::wstring _title;
			ref<Wnd> _wnd;
			bool _detached;
			bool _fullscreen;
		};

		wchar_t _hotkey;
		void DoContextMenu(int x, int y);
		std::vector<Pane> _panes;
		ref<Wnd> _current;
		int _headerHeight;

		enum {defaultHeaderHeight = 21};
};

#pragma warning(pop)

#endif