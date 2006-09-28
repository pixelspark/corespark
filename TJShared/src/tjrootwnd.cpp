#include "../include/tjshared.h"
using namespace tj::shared;

RootWnd::RootWnd(std::wstring title, const wchar_t* className, bool usedb): Wnd(title.c_str(),0, className, usedb) {
	SetStyleEx(WS_EX_CONTROLPARENT);
}

RootWnd::~RootWnd() {
}

void RootWnd::RenameWindow(ref<Wnd> w, std::wstring n) {
	if(!w) return;

	// rename in orphan panes
	std::vector< ref<Pane> >::iterator it = _orphans.begin();
	while(it!=_orphans.end()) {
		ref<Pane> pane = *it;
		if(pane->_wnd==w) {
			pane->_title = n;
			break;
		}
		it++;
	}

	// rename in floating panes
	std::vector< ref<FloatingPane> >::iterator ita = _floatingPanes.begin();
	while(ita!=_floatingPanes.end()) {
		ref<FloatingPane> pane = *ita;
		if(pane->_pane->_wnd==w) {
			pane->SetText(n.c_str());
			pane->_pane->_title = n;
			break;
		}
		ita++;
	}
}

void RootWnd::RemoveWindow(ref<Wnd> w) {
	if(!w) return;

	// remove from orphan panes
	std::vector< ref<Pane> >::iterator it = _orphans.begin();
	while(it!=_orphans.end()) {
		ref<Pane> pane = *it;
		if(pane->_wnd==w) {
			RemoveOrphanPane(pane);
			break;
		}
		it++;
	}

	// remove from floating panes
	std::vector< ref<FloatingPane> >::iterator ita = _floatingPanes.begin();
	while(ita!=_floatingPanes.end()) {
		ref<FloatingPane> pane = *ita;
		if(pane->_pane->_wnd==w) {
			RemoveFloatingPane(pane);
			break;
		}
		ita++;
	}

	// remove from tab windows
	std::vector< ref<TabWnd> >::iterator itb = _tabWindows.begin();
	while(itb!=_tabWindows.end()) {
		ref<TabWnd> tab = *itb;
		tab->RemovePane(w);
		itb++;
	}
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

void RootWnd::RemoveTabWindow(TabWnd* tw) {
	std::vector< ref<TabWnd> >::iterator it = _tabWindows.begin();
	while(it!=_tabWindows.end()) {
		ref<TabWnd> tab = *it;
		if(tab && tab.GetPointer()==tw) {
			_tabWindows.erase(it);
			return;
		}
		it++;
	}
}

void RootWnd::AddNotification(std::wstring text, std::wstring icon, int time) {
	int highestIndex = -1;
	std::vector< ref<NotificationWnd> >::iterator it = _notifications.begin();
	while(it!=_notifications.end()) {
		ref<NotificationWnd> wnd = *it;
		if(wnd) {
			highestIndex = max(wnd->GetIndex(), highestIndex);
		}
		it++;
	}
	ref<NotificationWnd> nw = GC::Hold(new NotificationWnd(text,icon,time, highestIndex+1, this));
	_notifications.push_back(nw);
}

void RootWnd::RemoveNotification(NotificationWnd* nw) {
	std::vector< ref<NotificationWnd> >::iterator it = _notifications.begin();
	while(it!=_notifications.end()) {
		ref<NotificationWnd> wnd = *it;
		if(wnd && wnd.GetPointer()==nw) {
			_notifications.erase(it);
			return;
		}
		it++;
	}
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

ref<TabWnd> RootWnd::FindTabWindowAt(int x, int y) {
	std::vector< ref<TabWnd> >::iterator itt = _tabWindows.begin();
	while(itt!=_tabWindows.end()) {
		ref<TabWnd> tab = *itt;
		tj::shared::Rectangle r = tab->GetWindowRectangle();
		if(r.IsInside(x,y)) {
			return tab;
		}
		itt++;
	}

	return 0;
}

void RootWnd::SetDragTarget(ref<TabWnd> tw) {
	if(_dragTarget) _dragTarget->Update();
	_dragTarget = tw;
	if(tw) tw->Update();
}

void RootWnd::Update() {
}

ref<TabWnd> RootWnd::GetDragTarget() {
	return _dragTarget;
}

void RootWnd::AddOrphanPane(ref<Pane> pane) {
	ref<Wnd> window = pane->GetWindow();
	if(window) {
		window->Show(false);
		SetParent(window->GetWindow(), _wnd);
		_orphans.push_back(pane);
	}
}

std::vector< ref<Pane> >* RootWnd::GetOrphanPanes() {
	return &_orphans;
}

void RootWnd::RemoveOrphanPane(ref<Pane> pane) {
	std::vector< ref<Pane> >::iterator it = _orphans.begin();
	while(it!=_orphans.end()) {
		ref<Pane> fp = *it;
		if(fp==pane) {
			_orphans.erase(it);
			return;
		}
		it++;
	}
}

/** AddNotificationRunnable */
AddNotificationRunnable::AddNotificationRunnable(RootWnd* root, std::wstring text, std::wstring icon, int time) {
	_root = root;
	_text = text;
	_icon = icon;
	_time = time;
}

AddNotificationRunnable::~AddNotificationRunnable() {

}

void AddNotificationRunnable::Run() {
	if(_root!=0) {
		_root->AddNotification(_text, _icon, _time);
	}
}