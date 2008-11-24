#ifndef _TJPOPUPWND_H
#define _TJPOPUPWND_H

namespace tj {
	namespace shared {
		class EXPORTED PopupWnd: public Wnd {
			public:
				PopupWnd(HWND parent = 0, bool isDialog = true);
				virtual ~PopupWnd();

				/* Popup windows automatically recalculate their size each time they 'pop up'.
				So theme+dpi switching will be handled correctly. */
				virtual void SetSize(Pixels x, Pixels y);
				virtual void SetOpacity(float f);
				virtual void PopupAt(Pixels clientX, Pixels clientY, ref<Wnd> window);
				virtual void PopupAtMouse();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Show(bool t);
				virtual void SetModal(bool m);

			protected:
				virtual void FitToMonitor(POINT& p);
				virtual void OnActivate(bool activate);
				Pixels _w, _h;
				bool _isModal;
		};
	}
}

#endif
