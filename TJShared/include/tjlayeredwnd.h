#ifndef _TJLAYEREDWND_H
#define _TJLAYEREDWND_H

class EXPORTED LayeredWnd: public Wnd {
	public:
		LayeredWnd(std::wstring title, HWND parent);
		virtual ~LayeredWnd();
		virtual void SetTransparency(float t);
};

#endif