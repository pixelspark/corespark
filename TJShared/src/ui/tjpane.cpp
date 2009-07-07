#include "../../include/ui/tjui.h" 
using namespace tj::shared;
using namespace tj::shared::graphics;

Placement::Placement(Type t, const std::wstring& container) {
	_type = t;
	_container = container;
}

Placement::~Placement() {
}

Pane::Pane(const std::wstring& title, ref<Wnd> window, bool detached, bool closable, ref<Settings> st, const Placement& p, std::wstring icon): _defaultPlacement(p) {
	_title = title;
	_wnd = window;
	_detached = detached;
	_closable = closable;
	_settings = st;

	if(icon.length()>0) {
		_icon = GC::Hold(new Icon(icon));
	}
};

Pane::~Pane() {
}

std::wstring Placement::TypeToString(Placement::Type t) {
	switch(t) {
		case Placement::Floating:
			return L"float";

		case Placement::Tab:
			return L"tab";

		case Placement::Orphan:
		default:
			return L"none";
	}
}

Placement::Type Placement::TypeFromString(const std::wstring& s) {
	if(s==L"float") {
		return Placement::Floating;
	}
	else if(s==L"tab") {
		return Placement::Tab;
	}
	else {
		return Placement::Orphan;
	}
}

void Pane::SetTitle(std::wstring c) {
	_title = c;
}

void Pane::OnMove(const Area& rc) {
	if(_settings) {
		_settings->SetValue(L"x", Stringify(rc.GetLeft()));
		_settings->SetValue(L"y", Stringify(rc.GetTop()));
		_settings->SetValue(L"w", Stringify(rc.GetWidth()));
		_settings->SetValue(L"h", Stringify(rc.GetHeight()));
	}
}

Area Pane::GetPreferredPosition() {
	ref<Wnd> paneWnd = GetWindow();
	Area wrc(0,0,0,0);
	if(paneWnd) {
		wrc = paneWnd->GetWindowArea();
	}

	if(_settings) {
		wrc.SetX(StringTo<Pixels>(_settings->GetValue(L"x", L"?"), wrc.GetLeft()));
		wrc.SetY(StringTo<Pixels>(_settings->GetValue(L"y", L"?"), wrc.GetTop()));
		wrc.SetWidth(StringTo<Pixels>(_settings->GetValue(L"w", L"?"), wrc.GetWidth()));
		wrc.SetHeight(StringTo<Pixels>(_settings->GetValue(L"h", L"?"), wrc.GetHeight()));
	}

	return wrc;
}

void Pane::OnPlacementChange(Placement& p) {
	if(_settings) {
		// Write new placement to settings
		_settings->SetValue(L"placement", Placement::TypeToString(p._type));
		_settings->SetValue(L"container", p._container);
	}
}
Placement Pane::GetPreferredPlacement() {
	if(_settings) {
		Placement p;
		p._type = Placement::TypeFromString(_settings->GetValue(L"placement", Placement::TypeToString(_defaultPlacement._type)));
		p._container = _settings->GetValue(L"container", _defaultPlacement._container);
		return p;
	}
	else {
		return _defaultPlacement;
	}
}

ref<Icon> Pane::GetIcon() const {
	// If the window specifies a tab icon, return that one instead
	ref<Icon> ni = _wnd->GetTabIcon();
	return ni?ni:_icon;
}

std::wstring Pane::GetTitle() const {
	std::wstring childTitle = _wnd->GetTabTitle();
	if(childTitle.length()>0) {
		if(_title.length()>0) {
			return _title + L": " + childTitle;
		}
		return childTitle;
	}
	else {
		return _title;
	}
}

bool Pane::IsClosable() const {
	return _closable;
}

ref<Wnd> Pane::GetWindow() {
	return _wnd;
}

const ref<Wnd> Pane::GetWindow() const {
	return _wnd;
}

bool Pane::HasIcon() const {
	return _icon!=0 || _wnd->GetTabIcon()!=0;
}

FloatingPane::FloatingPane(RootWnd* rw, ref<Pane> p): Wnd(null, false) {
	assert(p);
	_pane = p;
	_root = rw;
	
	#ifdef TJ_OS_WIN
		SetStyle(WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU|WS_CAPTION);
	#endif

	// adapt the window to this pane
	ref<Wnd> paneWnd = _pane->GetWindow();
	if(paneWnd) {
		Add(paneWnd);
		Area childArea = paneWnd->GetWindowArea();
		Move(childArea.GetLeft()+TabWnd::TearOffLimit, childArea.GetTop()+TabWnd::TearOffLimit, childArea.GetWidth(), childArea.GetHeight());
		paneWnd->Show(true);
	}
	
	// set window title
	SetText(p->GetTitle());

	Layout();
	Show(true);
}

FloatingPane::~FloatingPane() {
}

void FloatingPane::OnSize(const Area& ns) {
	Layout();
}

void FloatingPane::OnMove(const Area& rc) {
	_pane->OnMove(rc);
}

#ifdef TJ_OS_WIN
	LRESULT FloatingPane::Message(UINT msg, WPARAM wp, LPARAM lp) {
		if(msg==WM_CLOSE) {
			Show(false);
			if(_root) {
				if(!_pane->IsClosable()) {
					_root->AddOrphanPane(_pane);
				}
				_root->RemoveFloatingPane(_pane);
				return 0;
			}
		}
		else if(msg==WM_ENTERSIZEMOVE) {
			_dragging = true;
			Mouse::Instance()->SetCursorType(CursorSizeAll);
			_capture.StartCapturing(Mouse::Instance(), ref<Wnd>(this));
		}
		else if(msg==WM_EXITSIZEMOVE) {
			_dragging = false;
			Mouse::Instance()->SetCursorType(CursorDefault);
			// find tab window below this window and attach
			POINT p;
			GetCursorPos(&p);
			ref<TabWnd> below = _root->FindTabWindowAt(p.x, p.y);
			if(below) {
				below->Attach(_pane);
				_root->RemoveFloatingPane(_pane);
			}
			_root->SetDragTarget(null);
			_capture.StopCapturing();
			return 0;
		}
		else if(msg==WM_NCMOUSEMOVE||msg==WM_MOUSEMOVE) {
			if(_dragging) {
				POINT p;
				GetCursorPos(&p);
				ref<TabWnd> below = _root->FindTabWindowAt(p.x, p.y);
				_root->SetDragTarget(below);
				if(below) {
					below->Repaint();
				}
			}
		}
		else if(msg==WM_GETMINMAXINFO) {
			return _pane->GetWindow()->Message(msg,wp,lp);
		}
		else if(msg==WM_WINDOWPOSCHANGING) {
			bool isIcon = IsIconic(GetWindow()) == TRUE;
			bool isZoom = IsZoomed(GetWindow()) == TRUE;

			if(!isIcon && !isZoom) {
				WINDOWPOS* info = (LPWINDOWPOS)lp;
				if(info!=0) {
					RECT wndrc;
					wndrc.left = info->x;
					wndrc.top = info->y;
					wndrc.bottom = wndrc.top + info->cy;
					wndrc.right = wndrc.left + info->cx;
					HMONITOR monitor = MonitorFromRect(&wndrc, MONITOR_DEFAULTTONEAREST);

					MONITORINFO mi;
					const static int KSnapDistance = 12;
					mi.cbSize = sizeof(MONITORINFO);
					if(GetMonitorInfo(monitor, &mi)) {
						const RECT& rc = mi.rcWork;

						if(abs(info->x - rc.left) <= KSnapDistance) {
							// Snap left edge
							info->x = rc.left;
						}
						else if (abs((info->x + info->cx) - rc.right) <= KSnapDistance) {
							// Snap right edge
							info->x = rc.right - info->cx + 1;
						}

						if (abs(info->y - rc.top) <= KSnapDistance) {
							// Snap top edge
							info->y = rc.top;
						}
						else if(abs((info->y + info->cy) - rc.bottom) <= KSnapDistance) {
							// Snap bottom edge
							info->y = rc.bottom - info->cy + 1;
						}
					}
				}
			}
		}
		return Wnd::Message(msg,wp,lp);
	}
#endif

void FloatingPane::Layout() {
	Area rc = GetClientArea();
	ref<Wnd> child = _pane->GetWindow();
	if(child) {
		child->Fill(LayoutFill, rc);
	}
}


void FloatingPane::Paint(graphics::Graphics& g, strong<Theme> theme) {
}