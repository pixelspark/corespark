#ifndef _TJTOOLTIP_H
#define _TJTOOLTIP_H

namespace tj {
	namespace shared {
		class EXPORTED TooltipWnd: public virtual Object {
			public:
				TooltipWnd(HWND parent);
				virtual ~TooltipWnd();
				virtual void SetTrackEnabled(bool t);
				virtual void SetTrackPosition(int x, int y);
				virtual void SetTooltip(std::wstring text);

			protected:
				HWND _owner;
				HWND _wnd;
		};
	}
}

#endif