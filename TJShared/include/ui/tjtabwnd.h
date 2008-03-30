#ifndef _TJTAB_H
#define _TJTAB_H

namespace tj {
	namespace shared {
		class EXPORTED TabWnd: public ChildWnd {
			friend class FloatingPane;

			public:
				TabWnd(ref<WindowManager> root, const std::wstring& id = L"");
				virtual ~TabWnd();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				ref<Pane> AddPane(ref<Pane> pane, bool select = false);
				ref<Pane> GetPane(int index);
				void RemovePane(ref<Wnd> wnd);
				void SelectPane(unsigned int index);
				void SelectPane(ref<Wnd> wnd);
				virtual void Layout();
				virtual void Update();
				virtual void Clear();
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				void SetChildStyle(bool c);
				
				void Rename(ref<Wnd> pane, std::wstring name);
				ref<Wnd> GetCurrentPane();
				ref<Pane> GetPaneAt(int x);
				void Detach(ref<Pane> p);
				void Attach(ref<Pane> p);
				bool RevealWindow(ref<Wnd> w);
				void SetDetachAttachAllowed(bool allow);
				void SelectPane(ref<Pane> pane);
				virtual void Add(ref<Wnd> child); // do not use; use AddPane instead
				const std::wstring& GetID() const;
				Placement GetPlacement() const;
				virtual std::wstring GetTabTitle() const;
				virtual ref<Icon> GetTabIcon() const;
			
			protected:
				virtual void OnKey(Key k, wchar_t t, bool down, bool isAccelerator);
				virtual void OnFocus(bool focus);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);

				void SetDraggingPane(ref<Pane> pane);
				void DoAddMenu(Pixels x, Pixels y);
				void FixScrollerOffset();
				Pixels GetTotalTabWidth();
				
				std::vector< ref<Pane> > _panes;
				ref<Pane> _current;
				ref<Pane> _dragging;
				weak<WindowManager> _root;
				Pixels _headerHeight;
				Pixels _offset;
				int _dragStartX, _dragStartY;
				Icon _closeIcon;
				Icon _addIcon;
				bool _detachAttachAllowed;
				bool _childStyle;
				std::wstring _id;

				const static int TearOffLimit = 15;
				enum {defaultHeaderHeight = 24};
				const static int KIconWidth = 16;
				const static int KRealIconWidth = 14;
				const static Pixels KScrollerHeight = 3;
		};
	}
}

#endif
