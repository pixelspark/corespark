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
					EditingCommit,
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