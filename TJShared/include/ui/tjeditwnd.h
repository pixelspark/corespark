#ifndef _TJEDIT_H
#define _TJEDIT_H

namespace tj {
	namespace shared {
		class EXPORTED EditWnd: public ChildWnd {
			public:
				EditWnd();
				virtual ~EditWnd();
				virtual std::wstring GetText();
				void SetCue(std::wstring cue);
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Layout();
				virtual void SetText(std::wstring ws);

				struct NotificationTextChanged {
				};

				Listenable<NotificationTextChanged> EventTextChanged;

			protected:
				void UpdateColor();
				HFONT _font;
				HWND _ctrl;
				graphics::Color _back;
				HBRUSH _backBrush;
		};
	}
}

#endif