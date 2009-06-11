#ifndef _TJEDIT_H
#define _TJEDIT_H

namespace tj {
	namespace shared {
		class EXPORTED EditWnd: public ChildWnd {
			public:
				EditWnd(bool multiLine=false);
				virtual ~EditWnd();
				virtual String GetText();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void Layout();
				virtual void SetText(const String& ws);
				virtual void SetReadOnly(bool r);
				virtual void SetBorder(bool b);
				void SetCue(const String& cue);
				virtual bool HasFocus(bool childrenToo) const;
				virtual void Show(bool s);
				virtual void Focus();
				
				enum EditingType {
					EditingNone = 0,
					EditingStarted,
					EditingEnded,
					EditingTextChanged,
				};

				struct EXPORTED EditingNotification {
					EditingNotification(EditingType type);
					EditingType GetType() const;
					EditingType _type;
				};

				Listenable<EditingNotification> EventEditing;

			protected:
				EditWnd(bool multiLine, bool doubleBuffer);
				virtual void OnSize(const Area& ns);
				void UpdateColor();
				virtual void OnFocus(bool f);
			
				#ifdef TJ_OS_WIN
					virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				#endif

			private:
				void Create(bool multiline);

			protected:
				#ifdef TJ_OS_WIN
					HFONT _font;
					HWND _ctrl;
					HBRUSH _backBrush;
				#endif

				graphics::Color _back;
			};

		class EXPORTED SuggestionMenuItem: public MenuItem {
			public:
				SuggestionMenuItem(const String& value, const String& friendly, bool highlight = false, MenuItem::CheckType checked = MenuItem::NotChecked, const String& icon = L"", const String& hotkey = L"");
				virtual ~SuggestionMenuItem();
				virtual const String& GetSuggestionValue() const;

			protected:
				String _value;
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
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
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