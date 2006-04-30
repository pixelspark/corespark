#include "../include/tjshared.h"

RootWnd::RootWnd(std::wstring title): Wnd(title.c_str(),0, TJ_DEFAULT_CLASS_NAME, false) {
}

RootWnd::~RootWnd() {
}

LRESULT RootWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_ENTERMENULOOP) {
		return 0;
	}
	return Wnd::Message(msg,wp,lp);
}