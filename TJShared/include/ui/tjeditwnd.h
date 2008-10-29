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
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				void UpdateColor();
				virtual void OnFocus(bool f);

				HFONT _font;
				HWND _ctrl;
				graphics::Color _back;
				HBRUSH _backBrush;
		};
	}
}

#endif