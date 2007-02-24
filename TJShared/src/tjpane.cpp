#include "../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

Pane::Pane(std::wstring title, ref<Wnd> window, bool detached, bool closable, std::wstring icon) {
	_title = title;
	_wnd = window;
	_detached = detached;
	_closable = closable;
	if(icon.length()>0) {
		std::wstring path = ResourceManager::Instance()->Get(icon,true);
		_icon = Bitmap::FromFile(path.c_str(), TRUE);
	}
	else {
		_icon = 0;
	}
}

Pane::~Pane() {
	delete _icon;
}

void Pane::SetTitle(std::wstring c) {
	_title = c;
}

Gdiplus::Bitmap* Pane::GetIcon() {
	return _icon;
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

bool Pane::HasIcon() const {
	return _icon!=0;
}

FloatingPane::FloatingPane(RootWnd* rw, ref<Pane> p, TabWnd* source): Wnd(L"FloatingPane", 0, TJ_DEFAULT_CLASS_NAME, false) {
	assert(p);
	_pane = p;
	_root = rw;
	_source = source;
	SetStyleEx(WS_EX_PALETTEWINDOW);
	SetStyle(WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU|WS_CAPTION);

	// adapt the window to this pane
	ref<Wnd> paneWnd = _pane->GetWindow();
	RECT sz;
	GetWindowRect(paneWnd->GetWindow(), &sz);
	SetWindowPos(_wnd, 0, sz.left+TabWnd::TearOffLimit, sz.top+TabWnd::TearOffLimit, sz.right-sz.left, sz.bottom-sz.top, SWP_NOZORDER);
	SetParent(paneWnd->GetWindow(), _wnd);

	// set window title
	SetWindowText(_wnd, p->GetTitle().c_str());

	Layout();
	paneWnd->Show(true);
	Show(true);
}

FloatingPane::~FloatingPane() {
}

LRESULT FloatingPane::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE) {
		Layout();
	}
	else if(msg==WM_CLOSE) {
		ShowWindow(_wnd, SW_HIDE);
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
		SetCapture(_wnd);
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
		_root->SetDragTarget(0);
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
		GetClientRect(_wnd, &r);
		SetWindowPos(child, 0, 0,0, r.right-r.left, r.bottom-r.top, SWP_NOZORDER);
	}
}


void FloatingPane::Paint(Gdiplus::Graphics& g) {
}