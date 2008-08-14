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

void Pane::OnMove(Pixels x, Pixels y, Pixels w, Pixels h) {
	if(_settings) {
		_settings->SetValue(L"x", Stringify(x));
		_settings->SetValue(L"y", Stringify(y));
		_settings->SetValue(L"w", Stringify(w));
		_settings->SetValue(L"h", Stringify(h));
	}
}

RECT Pane::GetPreferredPosition() {
	RECT wrc;
	GetWindowRect(GetWindow()->GetWindow(), &wrc);

	if(_settings) {
		RECT rc;
		rc.left = StringTo<int>(_settings->GetValue(L"x", L"?"), wrc.left);
		rc.top = StringTo<int>(_settings->GetValue(L"y", L"?"), wrc.top);
		rc.right = rc.left + StringTo<int>(_settings->GetValue(L"w", L"?"), wrc.right-wrc.left);
		rc.bottom = rc.top + StringTo<int>(_settings->GetValue(L"h", L"?"), wrc.bottom-wrc.top);

		return rc;
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

FloatingPane::FloatingPane(RootWnd* rw, ref<Pane> p): Wnd(L"FloatingPane", 0, TJ_DEFAULT_CLASS_NAME, false) {
	assert(p);
	_pane = p;
	_root = rw;
	SetStyleEx(WS_EX_PALETTEWINDOW|WS_EX_CONTROLPARENT);
	SetStyle(WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU|WS_CAPTION);

	// adapt the window to this pane
	ref<Wnd> paneWnd = _pane->GetWindow();
	RECT sz;
	GetWindowRect(paneWnd->GetWindow(), &sz);
	SetWindowPos(GetWindow(), 0, sz.left+TabWnd::TearOffLimit, sz.top+TabWnd::TearOffLimit, sz.right-sz.left, sz.bottom-sz.top, SWP_NOZORDER);
	SetParent(paneWnd->GetWindow(), GetWindow());

	// set window title
	SetWindowText(GetWindow(), p->GetTitle().c_str());

	Layout();
	paneWnd->Show(true);
	Show(true);
}

FloatingPane::~FloatingPane() {
}

LRESULT FloatingPane::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE || msg==WM_MOVE) {
		if(msg==WM_SIZE) {
			Layout();
		}
		RECT rc;
		GetWindowRect(GetWindow(),&rc);
		_pane->OnMove(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
	}
	else if(msg==WM_CLOSE) {
		ShowWindow(GetWindow(), SW_HIDE);
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
		SetCursor(LoadCursor(0,IDC_SIZEALL));
		SetCapture(GetWindow());
	}
	else if(msg==WM_EXITSIZEMOVE) {
		_dragging = false;
		SetCursor(LoadCursor(0,IDC_ARROW));
		// find tab window below this window and attach
		POINT p;
		GetCursorPos(&p);
		ref<TabWnd> below = _root->FindTabWindowAt(p.x, p.y);
		if(below) {
			below->Attach(_pane);
			_root->RemoveFloatingPane(_pane);
		}
		_root->SetDragTarget(null);
		ReleaseCapture();
		return 0;
	}
	else if(msg==WM_NCMOUSEMOVE||msg==WM_MOUSEMOVE||WM_MOVING) {
		if(_dragging) {
			POINT p;
			GetCursorPos(&p);
			ref<TabWnd> below = _root->FindTabWindowAt(p.x, p.y);
			_root->SetDragTarget(below);
		}
	}
	else if(msg==WM_GETMINMAXINFO) {
		return _pane->GetWindow()->Message(msg,wp,lp);
	}
	return Wnd::Message(msg,wp,lp);
}

void FloatingPane::Layout() {
	HWND child = _pane->GetWindow()->GetWindow();
	if(child!=0) {
		RECT r;
		GetClientRect(GetWindow(), &r);
		SetWindowPos(child, 0, 0,0, r.right-r.left, r.bottom-r.top, SWP_NOZORDER);
	}
}


void FloatingPane::Paint(graphics::Graphics& g, ref<Theme> theme) {
}