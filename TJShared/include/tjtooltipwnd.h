#ifndef _TJTOOLTIPWND_H
#define _TJTOOLTIPWND_H

class EXPORTED TooltipWnd: public Wnd {
	public:
		TooltipWnd(HWND parent);
		virtual ~TooltipWnd();
		void AddTooltip(HWND wnd, std::wstring text, int x, int y, int w, int h, int uid=0);
		void RemoveTooltip(HWND wnd, int uid=0);
		virtual wchar_t GetPreferredHotkey();
		virtual void Paint(Gdiplus::Graphics& g);
		virtual void Layout();

	protected:
		void Fill();
};

#endif