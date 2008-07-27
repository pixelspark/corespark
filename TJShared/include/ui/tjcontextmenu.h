#ifndef _TJCONTEXTMENU_H
#define _TJCONTEXTMENU_H

namespace tj {
	namespace shared {
		class MenuItem;

		class EXPORTED Menu {
			public:
				virtual ~Menu();
				virtual void AddSeparator(const std::wstring& text = L"") = 0;
				virtual void AddItem(ref<MenuItem> ci) = 0;
		};

		/** The icon for a MenuItem needs to be 16x16 */
		class EXPORTED MenuItem {
			friend class ContextMenu;
			friend class ContextPopupWnd;

			public:
				enum CheckType {
					NotChecked = 0,
					Checked = 1,
					RadioChecked = 2,
				};

				MenuItem(); // separator
				MenuItem(const std::wstring& title, int command, bool highlight = false, CheckType checked = NotChecked, const std::wstring& icon = L"");
				MenuItem(const std::wstring& title, int command, bool highlight, CheckType checked, ref<Icon> icon);
				
				virtual ~MenuItem();
				virtual bool IsSeparator() const;
				virtual bool IsDisabled() const;
				virtual bool HasIcon() const;
				virtual void SetIcon(const std::wstring& icon);
				virtual bool IsLink() const;
				virtual void SetLink(bool l);
				virtual ref<Icon> GetIcon();
				virtual const std::wstring& GetTitle() const;

				virtual void SetTitle(const std::wstring& title);
				virtual void SetSeparator(bool s);


			protected:
				std::wstring _title;
				int _command;
				CheckType _checked;
				ref<Icon> _icon;
				bool _separator;
				bool _hilite;
				bool _link;
		};

		class EXPORTED ContextPopupWnd: public PopupWnd {
			public:
				ContextPopupWnd(ContextMenu* cm, HWND parent);
				virtual ~ContextPopupWnd();
				virtual int DoModal(ref<Wnd> parent, Pixels x, Pixels y); // return -1 when no command
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnKey(Key k, wchar_t ch, bool down, bool isAccelerator);

			protected:
				virtual void EndModal(int r);
				virtual void OnActivate(bool a);
				virtual void OnTimer(unsigned int id);
				int GetItemAt(Pixels y);

				// TODO: move some of these to Theme GetMeasure
				const static unsigned int KMaxItems;
				const static Pixels KItemHeight;
				const static Pixels KMinContextMenuWidth;

			private:
				ContextMenu* _cm;
				Animation _openAnimation;
				Animation _closeAnimation;
				int _result;
				ModalLoop _loop;
				Icon _checkedIcon, _radioCheckedIcon;
				int _mouseOver;
				int _mouseDown;
		};

		class EXPORTED ContextMenu: public Menu {
			friend class ContextPopupWnd;

			public:
				ContextMenu();
				virtual ~ContextMenu();
				int DoContextMenu(ref<Wnd> wnd, Pixels x, Pixels y);
				int DoContextMenu(ref<Wnd> wnd);
				virtual void AddItem(ref<MenuItem> ci);
				virtual void AddSeparator(const std::wstring& text = L"");
				virtual void AddItem(const std::wstring& name, int command, bool hilite = false, bool checked = false);
				virtual void AddItem(const std::wstring& name, int command, bool hilite, MenuItem::CheckType checked);

			protected:
				std::vector< ref<MenuItem> > _items;
				std::wstring _longestString;
		};
	}
}

#endif
