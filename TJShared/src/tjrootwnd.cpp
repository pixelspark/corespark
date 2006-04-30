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

ref<FloatingPane> RootWnd::AddFloatingPane(ref<Pane> p, TabWnd* source) {
	ref<FloatingPane> fp = GC::Hold(new FloatingPane(this, p, source));
	_floatingPanes.push_back(fp);
	return fp;
}

void RootWnd::RemoveFloatingPane(ref<Pane> pn) {
	std::vector< ref<FloatingPane> >::iterator it = _floatingPanes.begin();
	while(it!=_floatingPanes.end()) {
		ref<FloatingPane> fp = *it;
		if(fp->_pane==pn) {
			_floatingPanes.erase(it);
			return;
		}
		it++;
	}
}

void RootWnd::AddTabWindow(ref<TabWnd> wnd) {
	assert(wnd);
	_tabWindows.push_back(wnd);
}

void RootWnd::RemoveTabWindow(ref<TabWnd> wnd) {
	_tabWindows.erase(std::find(_tabWindows.begin(), _tabWindows.end(), wnd));
}

void RootWnd::RevealWindow(ref<Wnd> wnd) {
	assert(wnd);

	// find the window in the floating panes
	std::vector< ref<FloatingPane> >::iterator it = _floatingPanes.begin();
	while(it!=_floatingPanes.end()) {
		ref<FloatingPane> pane = *it;
		if(pane->_pane->GetWindow() == wnd) {
			pane->Show(true);
			SetForegroundWindow(pane->GetWindow());
			return;
		}
		it++;
	}

	// let the tab windows do their thing
	std::vector< ref<TabWnd> >::iterator itt = _tabWindows.begin();
	while(itt!=_tabWindows.end()) {
		ref<TabWnd> tab = *itt;
		if(tab->RevealWindow(wnd)) return;
		itt++;
	}
}