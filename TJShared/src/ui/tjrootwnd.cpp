#include "../../include/ui/tjui.h" 
#include <windowsx.h> 
using namespace tj::shared;
using namespace tj::shared::graphics;

const Pixels RootWnd::KStatusBarHeight = 18;

WindowManager::~WindowManager() {
}

RootWnd::RootWnd(std::wstring title, const wchar_t* className, bool usedb): TopWnd(title.c_str(),0, className, usedb), _grabberIcon(Icons::GetIconPath(Icons::IconGrabber)), _showStatusBar(true) {
	SetStyleEx(WS_EX_CONTROLPARENT);
}

RootWnd::~RootWnd() {
}

void RootWnd::SetShowStatusBar(bool s) {
	_showStatusBar = s;
	Repaint();
}

bool RootWnd::IsStatusBarShown() const {
	return _showStatusBar;
}

void RootWnd::RenameWindow(ref<Wnd> w, std::wstring n) {
	if(!w) return;

	// rename in orphan panes
	std::vector< ref<Pane> >::iterator it = _orphans.begin();
	while(it!=_orphans.end()) {
		ref<Pane> pane = *it;
		if(pane->GetWindow()==w) {
			pane->_title = n;
			break;
		}
		++it;
	}

	// rename in floating panes
	std::vector< ref<FloatingPane> >::iterator ita = _floatingPanes.begin();
	while(ita!=_floatingPanes.end()) {
		ref<FloatingPane> pane = *ita;
		if(pane->_pane->GetWindow()==w) {
			pane->SetText(n.c_str());
			pane->_pane->_title = n;
			break;
		}
		ita++;
	}
}

void RootWnd::Layout() {
}

Area RootWnd::GetClientArea() const {
	Area rc = Wnd::GetClientArea();
	if(IsStatusBarShown()) {
		rc.Narrow(0,0,0,KStatusBarHeight);
	}
	return rc;
}

void RootWnd::Paint(graphics::Graphics& g, ref<Theme> theme) {
	if(IsStatusBarShown()) {
		Area wrc = Wnd::GetClientArea();
		Area rc(wrc.GetLeft(), wrc.GetBottom()-KStatusBarHeight, wrc.GetWidth(), KStatusBarHeight);
		SolidBrush back(theme->GetColor(Theme::ColorBackground));
		g.FillRectangle(&back, rc);
		theme->DrawInsetRectangle(g, rc);

		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		GetWindowPlacement(GetWindow(), &wp);

		if(wp.showCmd!=SW_SHOWMAXIMIZED) {
			Area icon(rc.GetRight()-KStatusBarHeight, rc.GetTop(), KStatusBarHeight, KStatusBarHeight);
			g.DrawImage(_grabberIcon, icon);
		}
	}
}

void RootWnd::FullRepaint() {
	RECT rc;
	GetClientRect(GetWindow(), &rc);
	RedrawWindow(GetWindow(), &rc, 0, RDW_ALLCHILDREN|RDW_INTERNALPAINT|RDW_INVALIDATE|RDW_ERASE);
}

bool RootWnd::IsOrphanPane(ref<Wnd> w) {
	std::vector< ref<Pane> >::iterator it = _orphans.begin();
	while(it!=_orphans.end()) {
		ref<Pane> pane = *it;
		if(pane->GetWindow()==w) {
			return true;
		}
		++it;
	}

	return false;
}

void RootWnd::OnSize(const Area& ns) {
	Repaint();
}

void RootWnd::RemoveWindow(ref<Wnd> w) {
	if(!w) return;

	// remove from orphan panes
	std::vector< ref<Pane> >::iterator it = _orphans.begin();
	while(it!=_orphans.end()) {
		ref<Pane> pane = *it;
		if(pane->GetWindow()==w) {
			RemoveOrphanPane(pane);
			break;
		}
		++it;
	}

	// remove from floating panes
	std::vector< ref<FloatingPane> >::iterator ita = _floatingPanes.begin();
	while(ita!=_floatingPanes.end()) {
		ref<FloatingPane> pane = *ita;
		if(pane->_pane->GetWindow()==w) {
			RemoveFloatingPane(pane->_pane);
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
	else if(msg==WM_NCHITTEST) {
		if(IsStatusBarShown() && !IsFullScreen()) {
			WINDOWPLACEMENT wp;
			wp.length = sizeof(wp);
			GetWindowPlacement(GetWindow(), &wp);

			if(wp.showCmd!=SW_SHOWMAXIMIZED) {
				ref<Theme> theme = ThemeManager::GetTheme();
				float df = theme->GetDPIScaleFactor();
				POINT p;
				p.x = GET_X_LPARAM(lp);
				p.y = GET_Y_LPARAM(lp);
				ScreenToClient(GetWindow(), &p);
				Pixels px = Pixels(p.x/df);
				Pixels py = Pixels(p.y/df);

				Area rc = Wnd::GetClientArea();
				if(py > rc.GetBottom()-KStatusBarHeight && px > rc.GetRight()-KStatusBarHeight) {
					// status bar
					return HTBOTTOMRIGHT;
				}
			}
		}
	}
	
	return TopWnd::Message(msg,wp,lp);
}

ref<FloatingPane> RootWnd::AddFloatingPane(ref<Pane> p) {
	ref<FloatingPane> fp = GC::Hold(new FloatingPane(this, p));
	
	Placement np;
	np._type = Placement::Floating;
	p->OnPlacementChange(np);
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
		++it;
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
		++it;
	}
}

ref<TabWnd> RootWnd::GetTabWindowById(const std::wstring& id) {
	std::vector< ref<TabWnd> >::iterator it = _tabWindows.begin();
	while(it!=_tabWindows.end()) {
		ref<TabWnd> tw = *it;
		if(tw && tw->GetID()==id) {
			return tw;
		}
		++it;
	}
	return null;
}

void RootWnd::AddPane(ref<Pane> p, bool select) {
	Placement np = p->GetPreferredPlacement();
	if(np._type==Placement::Orphan) {
		AddOrphanPane(p);
	}
	else if(np._type==Placement::Tab) {
		ref<TabWnd> tab = GetTabWindowById(np._container);
		if(tab) {
			tab->AddPane(p, select);
		}
		else {
			// Add as orphan
			Log::Write(L"TJShared/RootWnd", L"Couldn't find container tab with id="+np._container);
			AddOrphanPane(p);
		}
	}
	else if(np._type==Placement::Floating) {
		RECT pos = p->GetPreferredPosition(); // Important: this has to go *before* the creation of the FloatingPane!
		ref<FloatingPane> fp = AddFloatingPane(p);

		// Update position from prefs, but only if the window fits on the visible part of the screen
		HMONITOR mon = MonitorFromRect(&pos, MONITOR_DEFAULTTONULL);
		if(mon!=NULL) {
			SetWindowPos(fp->GetWindow(), 0L, pos.left, pos.top, pos.right-pos.left, pos.bottom-pos.top, SWP_NOZORDER);
		}
	}
}

void RootWnd::RevealWindow(ref<Wnd> wnd, ref<TabWnd> addTo) {
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
		++it;
	}

	// let the tab windows do their thing
	std::vector< ref<TabWnd> >::iterator itt = _tabWindows.begin();
	while(itt!=_tabWindows.end()) {
		ref<TabWnd> tab = *itt;
		if(tab->RevealWindow(wnd)) return;
		++itt;
	}

	// probably an orphan pane; if we can find it, attach it to addTo if addTo!=0
	if(addTo) {
		std::vector< ref<Pane> >::iterator oit = _orphans.begin();
		while(oit!=_orphans.end()) {
			ref<Pane> orphan = *oit;
			if(orphan->GetWindow()==wnd) {
				_orphans.erase(oit);
				addTo->Attach(orphan);
				addTo->SelectPane(orphan);
				return;
			}
			++oit;
		}
	}
}

ref<TabWnd> RootWnd::FindTabWindowAt(int x, int y) {
	std::vector< ref<TabWnd> >::iterator itt = _tabWindows.begin();
	while(itt!=_tabWindows.end()) {
		ref<TabWnd> tab = *itt;
		Area r = tab->GetWindowArea();
		if(r.IsInside(x,y)) {
			return tab;
		}
		itt++;
	}

	return null;
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
		SetParent(window->GetWindow(), GetWindow());
		_orphans.push_back(pane);

		Placement np;
		np._type = Placement::Orphan;
		pane->OnPlacementChange(np);
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
		++it;
	}
}