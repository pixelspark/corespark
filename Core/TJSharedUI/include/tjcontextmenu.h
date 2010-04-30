/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJCONTEXTMENU_H
#define _TJCONTEXTMENU_H

namespace tj {
	namespace shared {
		class MenuItem;

		class EXPORTED Menu: public virtual Object {
			public:
				virtual ~Menu();
				virtual void AddSeparator(const String& text = L"") = 0;
				virtual void AddItem(strong<MenuItem> ci) = 0;
				virtual unsigned int GetItemCount() const = 0;
				virtual ref<MenuItem> GetItemByIndex(unsigned int idx) = 0;
				virtual Pixels GetLargestWidth(strong<Theme> theme, graphics::Font* fnt) const = 0;
		};

		/** The icon for a MenuItem needs to be 16x16 */
		class EXPORTED MenuItem: public virtual Object {
			friend class ContextMenu;
			friend class ContextPopupWnd;

			public:
				enum CheckType {
					NotChecked = 0,
					Checked = 1,
					RadioChecked = 2,
				};

				MenuItem(); // separator
				MenuItem(const String& title, int command, bool highlight = false, CheckType checked = NotChecked, const String& icon = L"", const String& hotkey = L"");
				MenuItem(const String& title, int command, bool highlight, CheckType checked, ref<Icon> icon);
				
				virtual ~MenuItem();
				virtual bool IsSeparator() const;
				virtual bool IsDisabled() const;
				virtual bool HasIcon() const;
				virtual void SetIcon(const String& icon);
				virtual void SetIcon(ref<Icon> icon);
				virtual void SetHotkey(const String& hk);
				virtual const String& GetHotkey() const;
				virtual bool HasHotkey() const;
				virtual bool IsLink() const;
				virtual void SetLink(bool l);
				virtual ref<Icon> GetIcon();
				virtual const String& GetTitle() const;

				virtual void SetTitle(const String& title);
				virtual void SetSeparator(bool s);

				virtual unsigned char GetIndent() const;
				virtual void SetIndent(unsigned char level);

				virtual int GetCommandCode() const;
				virtual void SetCommandCode(int c);
				virtual ref<Menu> GetSubMenu(); // returns null when this item doesn't have a submenu

			protected:
				String _title;
				String _hotkey;
				int _command;
				CheckType _checked;
				ref<Icon> _icon;
				bool _separator;
				bool _hilite;
				bool _link;
				unsigned char _indent;
		};

		class EXPORTED BasicMenu: public Menu {
			public:
				BasicMenu();
				virtual ~BasicMenu();
				virtual void AddSeparator(const String& text = L"");
				virtual void AddItem(strong<MenuItem> ci);
				virtual unsigned int GetItemCount() const;
				virtual ref<MenuItem> GetItemByIndex(unsigned int idx);
				virtual Pixels GetLargestWidth(strong<Theme> theme, graphics::Font* fnt) const;

			protected:
				std::vector< strong<MenuItem> > _items;
				String _longestString;
		};

		class EXPORTED SubMenuItem: public MenuItem, public BasicMenu {
			public:
				SubMenuItem();
				SubMenuItem(const String& title, bool highlight = false, CheckType checked = NotChecked, const String& icon = L"");
				SubMenuItem(const String& title, bool highlight = false, CheckType checked = NotChecked, ref<Icon> icon = null);
				virtual ~SubMenuItem();
				virtual ref<Menu> GetSubMenu();
		};

		class EXPORTED ContextPopupWnd: public PopupWnd {
			public:
				ContextPopupWnd(strong<Menu> menu, ref<Wnd> parent = null);
				virtual ~ContextPopupWnd();
				virtual ref<MenuItem> DoModal(strong<Wnd> parent, Pixels x, Pixels y); // returns null when no command
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);

			protected:
				virtual void EndModal(ref<MenuItem> result);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnKey(Key k, wchar_t ch, bool down, bool isAccelerator);
				virtual void OnActivate(bool a);
				virtual void OnTimer(unsigned int id);
				virtual void OnSize(const Area& ns);
				virtual void EnterSubMenu(strong<Menu> menu);
				virtual void LeaveSubMenu();
				virtual void OnSelectItem(strong<MenuItem> ci);
				virtual void DrawMenuItems(graphics::Graphics& g, strong<Theme> theme, strong<Menu> cm, const Area& rc);
				virtual void UpdateSize();
				int GetItemAt(Pixels y);
				strong<Menu> GetCurrentMenu();

				const static unsigned int KMaxItems;

			private:
				const static int KMouseOverNothing = -1;
				const static int KMouseOverBackButton = -2;

				strong<Menu> _firstMenu;
				std::deque< strong<Menu> > _menu;
				Animation _openAnimation;
				Animation _closeAnimation;
				ref<MenuItem> _resultItem;
				ModalLoop _loop;
				Icon _checkedIcon, _radioCheckedIcon, _subIcon;
				int _mouseOver;
				int _mouseDown;
		};

		/** ContextMenu is a wrapper class that can be created on the stack. It handles the menu creation and 
		popup stuff for context menus **/
		class EXPORTED ContextMenu {
			friend class ContextPopupWnd;

			public:
				ContextMenu();
				virtual ~ContextMenu();
				int DoContextMenu(ref<Wnd> wnd, Pixels x, Pixels y);
				ref<MenuItem> DoContextMenuByItem(ref<Wnd> wnd, Pixels x, Pixels y);
				int DoContextMenu(ref<Wnd> wnd);
				void AddItem(const String& name, int command, bool hilite = false, bool checked = false);
				void AddItem(const String& name, int command, bool hilite, MenuItem::CheckType checked);
				void AddItem(ref<MenuItem> ci);
				void AddSeparator(const String& text = L"");
				strong<Menu> GetMenu();

			protected:
				strong<Menu> _menu;
		};
	}
}

#endif
