#ifndef _TJTOOLTIP_H
#define _TJTOOLTIP_H

namespace tj {
	namespace shared {
		class EXPORTED TooltipWnd: public virtual Object {
			public:
				// TODO change this to TooltipWnd(ref<Wnd> parent);
				#ifdef TJ_OS_WIN
					TooltipWnd(HWND parent);
				#endif
			
				virtual ~TooltipWnd();
				virtual void SetTrackEnabled(bool t);
				virtual void SetTrackPosition(int x, int y);
				virtual void Move(Pixels x, Pixels y);
				virtual void SetTooltip(String text);

			protected:
				#ifdef TJ_OS_WIN
					HWND _owner;
					HWND _wnd;
				#endif
		};
	}
}

#endif