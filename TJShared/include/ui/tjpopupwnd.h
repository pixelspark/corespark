#ifndef _TJPOPUPWND_H
#define _TJPOPUPWND_H

namespace tj {
	namespace shared {

		class EXPORTED PopupWnd: public Wnd {
			public:
				PopupWnd();
				virtual ~PopupWnd();

				/* Popup windows automatically recalculate their size each time they 'pop up'.
				So theme+dpi switching will be handled correctly. */
				virtual void SetSize(Pixels x, Pixels y);
				virtual void SetOpacity(float f);
				virtual void PopupAt(Pixels clientX, Pixels clientY, ref<Wnd> window);
				virtual void PopupAtMouse();

			protected:
				virtual void OnActivate(bool activate);
				Pixels _w, _h;
		};
	}
}

#endif