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
