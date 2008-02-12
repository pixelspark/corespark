#ifndef _TJCONTEXTMENU_H
#define _TJCONTEXTMENU_H

namespace tj {
	namespace shared {
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
				ContextItem(const std::wstring& title, int command, bool highlight = false, CheckType checked = NotChecked);
				~ContextItem();
				bool IsSeparator() const;
				bool IsDisabled() const;

			protected:
				std::wstring _title;
				int _command;
				bool _hilite;
				CheckType _checked;
				bool _separator;
		};

		
		class EXPORTED ContextPopupWnd: public PopupWnd {
			public:
				ContextPopupWnd(ContextMenu* cm, ref<Wnd> parent);
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
				void AddSeparator();

			protected:
				std::vector<ContextItem> _items;
				std::wstring _longestString;
				const static Pixels KItemHeight;
				const static Pixels KMinContextMenuWidth;
		};
	}
}

#endif