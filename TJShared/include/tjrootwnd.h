#ifndef _TJROOTWND_H
#define _TJROOTWND_H

class EXPORTED RootWnd: public Wnd {
	public:
		RootWnd(std::wstring title);
		virtual ~RootWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
};

#endif