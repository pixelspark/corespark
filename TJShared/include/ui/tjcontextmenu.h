#ifndef _TJCONTEXTMENU_H
#define _TJCONTEXTMENU_H

namespace tj {
	namespace shared {
		/** The icon for a ContextItem needs to be 16x16 */
		class EXPORTED ContextItem {
			friend class ContextMenu;
			friend class ContextPopupWnd;

			public:
				enum CheckType {
					NotChecked = 0,
					Checked = 1,
					RadioChecked = 2,
				};

				ContextItem(); // separator
				ContextItem(const std::wstring& title, int command, bool highlight = false, CheckType checked = NotChecked, const std::wstring& icon = L"");
				virtual ~ContextItem();
				virtual bool IsSeparator() const;
				virtual bool IsDisabled() const;
				virtual bool HasIcon() const;
				virtual void SetIcon(const std::wstring& icon);
				virtual bool IsLink() const;
				virtual void SetLink(bool l);
				virtual Icon* GetIcon();
				virtual const std::wstring& GetTitle() const;

			protected:
				std::wstring _title;
				int _command;
				CheckType _checked;
				Icon* _icon;
				bool _separator;
				bool _hilite;
				bool _link;
		};

		class EXPORTED ContextPopupWnd: public PopupWnd {
			public:
				ContextPopupWnd(ContextMenu* cm, HWND parent);
				virtual ~ContextPopupWnd();
				virtual int DoModal(); // return -1 when no command
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnKey(Key k, wchar_t ch, bool down);

			protected:
				virtual void EndModal(int r);
				virtual void OnActivate(bool a);
				ContextMenu* _cm;
				int _result;
				ModalLoop _loop;
				Icon _checkedIcon, _radioCheckedIcon;
				int _mouseOver;
				int _mouseDown;
		};

		class EXPORTED ContextMenu {
			friend class ContextPopupWnd;

			public:
				ContextMenu();
				virtual ~ContextMenu();
				int DoContextMenu(ref<Wnd> wnd, Pixels x, Pixels y);
				int DoContextMenu(ref<Wnd> wnd);
				void AddItem(const std::wstring& name, int command, bool hilite = false, bool checked = false);
				void AddItem(const std::wstring& name, int command, bool hilite, ContextItem::CheckType checked);
				void AddItem(ref<ContextItem> ci);
				void AddSeparator();

			protected:
				std::vector< ref<ContextItem> > _items;
				std::wstring _longestString;
				const static Pixels KItemHeight;
				const static Pixels KMinContextMenuWidth;
		};
	}
}

#endif
