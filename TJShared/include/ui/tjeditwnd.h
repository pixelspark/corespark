#ifndef _TJEDIT_H
#define _TJEDIT_H

namespace tj {
	namespace shared {
		class EXPORTED EditWnd: public ChildWnd {
			public:
				EditWnd(HWND parent);
				virtual ~EditWnd();
				virtual std::wstring GetText();
				void SetCue(std::wstring cue);
				virtual void Paint(Gdiplus::Graphics& g);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Layout();
				virtual void SetText(std::wstring ws);

			protected:
				void UpdateColor();
				HFONT _font;
				HWND _ctrl;
				Gdiplus::Color _back;
				HBRUSH _backBrush;
		};
	}
}

#endif