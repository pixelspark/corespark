#ifndef _TJEDIT_H
#define _TJEDIT_H

namespace tj {
	namespace shared {
		class EXPORTED EditWnd: public ChildWnd {
			public:
				EditWnd(bool multiLine=false);
				virtual ~EditWnd();
				virtual std::wstring GetText();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void Layout();
				virtual void SetText(const std::wstring& ws);
				virtual void SetReadOnly(bool r);
				virtual void SetBorder(bool b);
				void SetCue(const std::wstring& cue);
				virtual bool HasFocus(bool childrenToo) const;
				virtual void Show(bool s);
				virtual void Focus();

				struct NotificationTextChanged {
				};

				Listenable<NotificationTextChanged> EventTextChanged;

			protected:
				EditWnd(bool multiLine, bool doubleBuffer);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				void UpdateColor();
				virtual void OnFocus(bool f);

			private:
				void Create(bool multiline);

			protected:
				HFONT _font;
				HWND _ctrl;
				graphics::Color _back;
				HBRUSH _backBrush;
		};

		class EXPORTED SuggestionMenuItem: public MenuItem {
			public:
				SuggestionMenuItem(const std::wstring& value, const std::wstring& friendly, bool highlight = false, MenuItem::CheckType checked = MenuItem::NotChecked, const std::wstring& icon = L"", const std::wstring& hotkey = L"");
				virtual ~SuggestionMenuItem();
				virtual const std::wstring& GetSuggestionValue() const;

			protected:
				std::wstring _value;
		};

		class EXPORTED SuggestionEditWnd: public EditWnd {
			public:
				enum SuggestionMode {
					SuggestionModeReplace = 1,
					SuggestionModeInsert,
				};

				SuggestionEditWnd(bool multiLine = false);
				virtual ~SuggestionEditWnd();
				virtual void Layout();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual strong<Menu> GetSuggestionMenu();
				virtual ref<MenuItem> DoSuggestionMenu();

				virtual void SetSuggestionMode(SuggestionMode sm);
				virtual SuggestionMode GetSuggestionMode() const;

			protected:
				Icon _arrowIcon;
				ref<ContextMenu> _cm;
				SuggestionMode _sm;
		};
	}
}

#endif