#ifndef _TJPOPUPWND_H
#define _TJPOPUPWND_H

namespace tj {
	namespace shared {
		class EXPORTED PopupWnd: public Wnd, public Animatable {
			public:
				PopupWnd(ref<Wnd> parent = 0, bool isDialog = true);
				virtual ~PopupWnd();

				/* Popup windows automatically recalculate their size each time they 'pop up'.
				So theme+dpi switching will be handled correctly. */
				virtual void SetSize(Pixels x, Pixels y);
				virtual void SetOpacity(float f);
				virtual void PopupAt(Pixels clientX, Pixels clientY, ref<Wnd> window);
				virtual void PopupAtMouse();
				virtual void Show(bool t);
				virtual void SetModal(bool m);

			protected:
				virtual void OnAnimationStep(const Animated& member);

				#ifdef TJ_OS_WIN
					virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
					virtual void FitToMonitor(POINT& p);
				#endif
				virtual void OnActivate(bool activate);
				Animated _w; // Pixels
				Animated _h; // Pixels
				bool _isModal;
		};
	}
}

#endif
