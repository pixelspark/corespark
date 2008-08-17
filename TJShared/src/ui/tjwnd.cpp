#include "../../include/ui/tjui.h" 
#include <commctrl.h>
#include <shellapi.h>
#include <windowsx.h>
#define ISVKKEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000))

using namespace tj::shared;
using namespace tj::shared::graphics;

bool Wnd::_classesRegistered = false;

/* GDI+ Init */
GraphicsInit::GraphicsInit() {
	graphics::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	graphics::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	INITCOMMONCONTROLSEX sex;
	sex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	sex.dwICC = ICC_STANDARD_CLASSES|ICC_TAB_CLASSES|ICC_PROGRESS_CLASS|ICC_UPDOWN_CLASS|ICC_USEREX_CLASSES|ICC_WIN95_CLASSES;
	InitCommonControlsEx(&sex);
}

GraphicsInit::~GraphicsInit() {
}

Wnd::Wnd(const wchar_t* title, HWND parent, const wchar_t* className, bool usedb, int exStyle): _horizontalPos(0), _verticalPos(0), _horizontalPageSize(1), _verticalPageSize(1), _buffer(0), _doubleBuffered(usedb) {
	RegisterClasses();
	_wnd = CreateWindowEx(exStyle, className, title, WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, (HMENU)0, GetModuleHandle(NULL), (void*)this);
	if(_wnd==0) Throw(L"Could not create window", ExceptionTypeError);

	UpdateWindow(_wnd);
}

void Wnd::SetText(const wchar_t* t) {
	SetWindowText(_wnd, t);
}

bool Wnd::IsKeyDown(Key k) {
	int vk = 0;
	switch(k) {
		case KeyLeft:
			vk = VK_LEFT;
			break;

		case KeyRight:
			vk = VK_RIGHT;
			break;

		case KeyUp:
			vk = VK_UP;
			break;

		case KeyDown:
			vk = VK_DOWN;
			break;

		case KeyControl:
			vk = VK_CONTROL;
			break;

		case KeyShift:
			vk = VK_SHIFT;
			break;

		case KeyPageUp:
			vk = VK_PRIOR;
			break;

		case KeyPageDown:
			vk = VK_NEXT;
			break;

		case KeyDelete:
			vk = VK_DELETE;
			break;

		case KeyInsert:
			vk = VK_INSERT;
			break;

		case KeyMouseLeft: {
			bool swap = (GetSystemMetrics(SM_SWAPBUTTON)==TRUE);
			vk = swap?VK_RBUTTON:VK_LBUTTON;
			break;
		}

		case KeyMouseRight: {
			bool swap = (GetSystemMetrics(SM_SWAPBUTTON)==TRUE);
			vk = swap?VK_LBUTTON:VK_RBUTTON;
			break;
		}

		case KeyMouseMiddle:
			vk = VK_MBUTTON;
			break;

		default:
			return false;
	}

	return (GetAsyncKeyState(vk) & 0x8000) == 0x8000;
}

void Wnd::StartTimer(Time t, unsigned int id) {
	::SetTimer(_wnd, id, t.ToInt(), NULL);
}

void Wnd::StopTimer(unsigned int id) {
	::KillTimer(_wnd, id);
}

void Wnd::Update() {
	//Repaint();
}

void Wnd::Layout() {
}

void Wnd::SetSettings(ref<Settings> st) {
	_settings = st;
	if(_settings) {
		OnSettingsChanged();
	}
}

ref<Settings> Wnd::GetSettings() {
	return _settings;
}

void Wnd::OnSettingsChanged() {
}

void Wnd::SetDropTarget(bool t) {
	DragAcceptFiles(_wnd, (BOOL)t);
}

void Wnd::OnDropFiles(const std::vector< std::wstring >& files) {
	// Do nothing
}

void Wnd::OnContextMenu(Pixels x, Pixels y) {
	// Do nothing
}

void Wnd::Add(ref<Wnd> child, bool shown) {
	if(child) {
		SetParent(child->GetWindow(), GetWindow());
		child->SetStyle(WS_CHILD);
		child->Show(shown);
		SetStyleEx(WS_EX_CONTROLPARENT);
	}
}

void Wnd::Focus() {
	::SetFocus(_wnd);
}

void Wnd::BringToFront() {
	::SetForegroundWindow(_wnd);
}

void Wnd::Show(bool t) {
	ShowWindow(_wnd, t?SW_SHOW:SW_HIDE);
	if(!t) {
		delete _buffer;
		_buffer = 0;
	}
}

bool Wnd::HasFocus(bool childrenToo) const {
	HWND focus = GetFocus();
	return GetFocus()==_wnd || (childrenToo && IsChild(_wnd, focus));
}

bool Wnd::IsShown() const {
	LONG s = GetWindowLong(_wnd, GWL_STYLE);
	return (s&WS_VISIBLE)>0;
}

HWND Wnd::GetWindow() {
	return _wnd;
}

Wnd::~Wnd() {
	DestroyWindow(_wnd);
	delete _buffer;
}

void Wnd::Repaint() {
	RECT r;
	GetClientRect(_wnd, &r);
	InvalidateRect(_wnd, &r, FALSE);
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch(msg) {
		case WM_DESTROY:
			return 0;

		case WM_NCCREATE: {
			CREATESTRUCT* cs = (CREATESTRUCT*)lp;
			if(cs!=0) {
				SetWindowLong(wnd, GWL_USERDATA, (long)reinterpret_cast<long long>(cs->lpCreateParams));
			}
			return DefWindowProc(wnd, msg, wp, lp);
		}
		break;

		default: {
			Wnd* dp = reinterpret_cast<Wnd*>((long long)GetWindowLong(wnd,GWL_USERDATA));

			if(dp!=0) {
				return dp->PreMessage(msg,wp,lp);
			}

			return DefWindowProc(wnd, msg, wp, lp);
		}
		break;
	}
}

void Wnd::RegisterClasses() {
	if(_classesRegistered) return;

	WNDCLASSEX wc;
	memset(&wc,0,sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = TJ_DEFAULT_CLASS_NAME;
	wc.style = CS_DBLCLKS;
	
	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class",ExceptionTypeError);
	}

	wc.style |= CS_DROPSHADOW;
	wc.lpszClassName = TJ_DROPSHADOW_CLASS_NAME;
	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class",ExceptionTypeError);
	}

	_classesRegistered = true;
}

Wnd* Wnd::GetParent() {
	HWND parent = ::GetParent(_wnd);
	if(parent!=0) {
		Wnd* wp = reinterpret_cast<Wnd*>((long long)GetWindowLong(parent,GWL_USERDATA));
		if(wp!=0) return wp;
	}

	return 0;
}

Wnd* Wnd::GetRootWindow() {
	HWND parent = ::GetWindow(_wnd, GW_OWNER);
	if(parent!=0) {
		Wnd* wp = reinterpret_cast<Wnd*>((long long)GetWindowLong(parent,GWL_USERDATA));
		if(wp!=0) return wp;
	}

	return 0;
}

bool Wnd::IsMouseOver() {
	POINT cursorPosition;
	GetCursorPos(&cursorPosition);

	RECT rc;
	GetWindowRect(_wnd, &rc);
	
	return (cursorPosition.x > rc.left && cursorPosition.y > rc.top && cursorPosition.x < rc.right && cursorPosition.y < rc.bottom);
}

void Wnd::SetStyle(DWORD s) {
	DWORD wcs = GetWindowLong(_wnd,GWL_STYLE);
	wcs |= s;
	SetWindowLong(_wnd,GWL_STYLE,wcs);
}

void Wnd::UnsetStyle(DWORD s) {
	DWORD wcs = GetWindowLong(_wnd,GWL_STYLE);
	wcs &= (~s);
	SetWindowLong(_wnd,GWL_STYLE,wcs);
}

void Wnd::SetStyleEx(DWORD s) {
	DWORD wcs = GetWindowLong(_wnd,GWL_EXSTYLE);
	wcs |= s;
	SetWindowLong(_wnd,GWL_EXSTYLE,wcs);
}

void Wnd::UnsetStyleEx(DWORD s) {
	DWORD wcs = GetWindowLong(_wnd,GWL_EXSTYLE);
	wcs &= (~s);
	SetWindowLong(_wnd,GWL_EXSTYLE,wcs);
}

void Wnd::SetVerticallyScrollable(bool s) {
	s?SetStyle(WS_VSCROLL):UnsetStyle(WS_VSCROLL);
}

void Wnd::SetHorizontallyScrollable(bool s) {
	s?SetStyle(WS_HSCROLL):UnsetStyle(WS_HSCROLL);
}

int Wnd::GetHorizontalPos() {
	return _horizontalPos;
}

int Wnd::GetVerticalPos() {
	return _verticalPos;
}

void Wnd::SetVerticalPos(int p) {
	SCROLLINFO si;
	memset(&si,0,sizeof(SCROLLINFO));
	si.fMask = SIF_POS;
	si.nPos = p;
	SetScrollInfo(_wnd, SB_VERT, &si, TRUE);
	_verticalPos = p;
}

void Wnd::SetHorizontalPos(int p) {
	SCROLLINFO si;
	memset(&si,0,sizeof(SCROLLINFO));
	si.fMask = SIF_POS;
	si.nPos = p;
	SetScrollInfo(_wnd, SB_HORZ, &si, TRUE);
	_horizontalPos = p;
}

void Wnd::SetHorizontalScrollInfo(Range<int> rng, int pageSize) {
	SCROLLINFO srl;
	memset(&srl,0,sizeof(SCROLLINFO));
	srl.cbSize = sizeof(SCROLLINFO);
	srl.fMask = SIF_RANGE|SIF_PAGE;
	srl.nMax = rng.End();
	srl.nMin = rng.Start();
	_horizontalPageSize = pageSize;
	srl.nPage = pageSize;
	
	SetScrollInfo(_wnd, SB_HORZ,&srl,TRUE);

	int pos = GetHorizontalPos();
	if(pos > (rng.End()-pageSize) || pos < rng.Start()) {
		SetHorizontalPos(0);
	}
}

void Wnd::SetVerticalScrollInfo(Range<int> rng, int pageSize) {
	SCROLLINFO srl;
	memset(&srl,0,sizeof(SCROLLINFO));
	srl.cbSize = sizeof(SCROLLINFO);
	srl.fMask = SIF_RANGE|SIF_PAGE;
	srl.nMax = rng.End();
	srl.nMin = rng.Start();
	srl.nPage = pageSize;

	_verticalPageSize = pageSize;
	SetScrollInfo(_wnd, SB_VERT,&srl,TRUE);

	int pos = GetVerticalPos();
	if(pos > (rng.End()-pageSize) || pos < rng.Start()) {
		SetVerticalPos(0);
	}
}

LRESULT ColorWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	return Wnd::Message(msg,wp,lp);
}

void ColorWnd::Paint(graphics::Graphics& g, ref<Theme> theme) {
	SolidBrush br(Color(_r, _g, _b));
	g.FillRectangle(&br, GetClientArea());
}

void Wnd::Move(Pixels x, Pixels y, Pixels w, Pixels h) {
	ref<Theme> theme = ThemeManager::GetTheme();
	float df = theme->GetDPIScaleFactor();

	MoveWindow(_wnd, int(ceil(x*df)), int(ceil(y*df)), int(ceil(w*df)), int(ceil(h*df)), TRUE);
}

LRESULT Wnd::PreMessage(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_ERASEBKGND) {
		return 1;
	}
	else if(msg==WM_PAINT) {
		int style = GetWindowLong(_wnd, GWL_STYLE);
		if((style & WS_VISIBLE) == 0) {
			return 0;
		}

		PAINTSTRUCT ps;
		BeginPaint(_wnd, &ps);
		Graphics org(ps.hdc);

		ref<Theme> theme = ThemeManager::GetTheme();
		float dpiScale = theme->GetDPIScaleFactor();

		if(!_doubleBuffered) {
			GraphicsContainer container = org.BeginContainer();
			org.ScaleTransform(dpiScale, dpiScale);
			Paint(org, theme);
			org.EndContainer(container);
		}
		else {
			RECT cw;
			GetClientRect(_wnd, &cw);

			if(_buffer==0 || int(_buffer->GetWidth()) < int(cw.right-cw.left) || int(_buffer->GetHeight()) < int(cw.bottom-cw.top)) {
				delete _buffer;
				_buffer = 0;
				_buffer = new Bitmap(cw.right-cw.left+100, cw.bottom-cw.top+100, &org); // +100 for buffer
			}

			Graphics buffered(_buffer);
			GraphicsContainer container = buffered.BeginContainer();
			buffered.ScaleTransform(dpiScale, dpiScale);
			Paint(buffered, theme);
			buffered.EndContainer(container);
			org.DrawImage(_buffer,0,0);
		}

		EndPaint(_wnd, &ps);
		return 0;
	}	
	else {
		return Message(msg, wp, lp);
	}
}

void Wnd::OnKey(Key k, wchar_t ch, bool down, bool accelerator) {
}

LRESULT Wnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CLOSE) {
		Show(false);
		return 0;
	}
	else if(msg==WM_TIMER) {
		OnTimer((unsigned int)wp);
		return 0;
	}
	else if(msg==WM_CONTEXTMENU) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		POINT p;
		p.x = GET_X_LPARAM(lp);
		p.y = GET_Y_LPARAM(lp);
		ScreenToClient(GetWindow(), &p);
		OnContextMenu(Pixels(p.x/df), Pixels(p.y/df));
	}
	else if(msg==WM_APPCOMMAND) {
		int c = GET_APPCOMMAND_LPARAM(lp);

		switch(c) {
			case APPCOMMAND_BROWSER_BACKWARD:
				OnKey(KeyBrowseBack, 0, true, false);
				break;

			case APPCOMMAND_BROWSER_FORWARD:
				OnKey(KeyBrowseForward, 0, true, false);
				break;
		}
	}
	// OnMouse handlers
	else if(msg==WM_LBUTTONDOWN) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventLDown, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_RBUTTONDOWN) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventRDown, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_MBUTTONDOWN) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventMDown, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_LBUTTONUP) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventLUp, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_MBUTTONUP) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventMUp, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_RBUTTONUP) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventRUp, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_LBUTTONDBLCLK) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventLDouble, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_RBUTTONDBLCLK) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventRDouble, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_MOUSEMOVE) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventMove, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		SetWantMouseLeave(_wantsMouseLeave);
		return 0;
	}
	else if(msg==WM_MOUSELEAVE) {
		ref<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		OnMouse(MouseEventLeave, int(ceil(GET_X_LPARAM(lp)/df)), int(ceil(GET_Y_LPARAM(lp)/df)));
		return 0;
	}
	else if(msg==WM_MOUSEWHEEL) {
		int d = GET_WHEEL_DELTA_WPARAM(wp);
		if(d<0) {
			SendMessage(_wnd, WM_VSCROLL, MAKELONG(SB_LINEDOWN, 0), 0L); 
		}
		else {
			SendMessage(_wnd, WM_VSCROLL, MAKELONG(SB_LINEUP, 0), 0L); 
		}
	}
	else if(msg==WM_ACTIVATE) {
		bool activate = LOWORD(wp)!=WA_INACTIVE;
		OnActivate(activate);
		return 0;
	}
	else if(msg==WM_DROPFILES) {
		std::vector<std::wstring> files;

		HDROP drop = (HDROP)wp;
		int n = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);
		for(int a=0;a<n;a++) {
			int size = DragQueryFile(drop, a, NULL, 0);
			wchar_t* buffer = new wchar_t[size+2];
			DragQueryFile(drop, a, buffer, size+1);
			files.push_back(std::wstring(buffer));
			delete[] buffer;
		}
		DragFinish(drop);
		OnDropFiles(files);
	}
	else if(msg==WM_SIZE) {
		Area size = GetClientArea();

		if(!(size.GetWidth()==0 || size.GetHeight()==0)) {
			/* window is hidden, don't bother changing layout
			   Some windows add scrollbars when the area is very small (0,0)
			   and this prevents them from doing so */
			OnSize(size);
		}
		return 0;
	}
	else if(msg==WM_HSCROLL) {
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
		GetScrollInfo(_wnd, SB_HORZ, &si);
		
		if(LOWORD(wp)==SB_THUMBTRACK||LOWORD(wp)==SB_THUMBPOSITION) { 
			_horizontalPos = si.nTrackPos;
			SetScrollPos(_wnd,SB_HORZ, _horizontalPos,TRUE);
		}
		else {
			if(LOWORD(wp)==SB_PAGERIGHT) {
				_horizontalPos += _horizontalPageSize;
			}
			else if(LOWORD(wp)==SB_PAGELEFT) {
				_horizontalPos -= _horizontalPageSize;		
			}
			else if(LOWORD(wp)==SB_LINERIGHT) {
				++_horizontalPos;
			}
			else if(LOWORD(wp)==SB_LINELEFT) {
				--_horizontalPos;
			}

			if(_horizontalPos<si.nMin) _horizontalPos = si.nMin;
			if(_horizontalPos>(si.nMax-int(si.nPage))) _horizontalPos = si.nMax-int(si.nPage);
			SetScrollPos(_wnd,SB_HORZ, _horizontalPos,TRUE);
		}

		OnScroll(ScrollDirectionHorizontal);
		Repaint();
		return 0;
	}
	else if(msg==WM_VSCROLL) {
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
		GetScrollInfo(_wnd, SB_VERT, &si);

		if(LOWORD(wp)==SB_THUMBTRACK||LOWORD(wp)==SB_THUMBPOSITION) {
			_verticalPos = si.nTrackPos;
			SetScrollPos(_wnd,SB_VERT, _verticalPos,TRUE);
		}
		else {
			if(LOWORD(wp)==SB_PAGERIGHT) {
				_verticalPos += _verticalPageSize;
			}
			else if(LOWORD(wp)==SB_PAGELEFT) {
				_verticalPos -= _verticalPageSize;		
			}
			else if(LOWORD(wp)==SB_LINERIGHT) {
				++_verticalPos;		
			}
			else if(LOWORD(wp)==SB_LINELEFT) {
				--_verticalPos;
			}

			if(_verticalPos<si.nMin) _verticalPos = si.nMin;
			if(_verticalPos>(si.nMax-int(si.nPage))) _verticalPos = si.nMax-int(si.nPage);
			SetScrollPos(_wnd,SB_VERT, _verticalPos,TRUE);
		}

		OnScroll(ScrollDirectionVertical);
		Repaint();
		return 0;
	}
	else if(msg==WM_SETFOCUS) {
		OnFocus(true);
	}
	else if(msg==WM_KILLFOCUS) {
		OnFocus(false);
	}
	else if(msg==WM_KEYDOWN || msg==WM_KEYUP || msg==WM_SYSKEYDOWN || msg==WM_SYSKEYUP) {
		if(msg==WM_KEYDOWN && IsKeyDown(KeyControl)) {
			if(wp==L'C'||wp==L'c') {
				OnCopy();
			}
			else if(wp==L'V'||wp==L'v') {
				OnPaste();
			}
			else if(wp==L'X'||wp==L'x') {
				OnCut();
			}
		}

		Key key = KeyNone;
		wchar_t ch = L'\0';
		TranslateKeyCodes((int)wp, key, ch);
		OnKey(key, ch, msg==WM_KEYDOWN, (msg==WM_SYSKEYUP || msg==WM_SYSKEYDOWN));
	}
	else if(msg==WM_COPY) {
		OnCopy();
	}
	else if(msg==WM_PASTE) {
		OnPaste();
	}
	else if(msg==WM_CUT) {
		OnCut();
	}
	else if(msg==WM_NOTIFY) {
		// This is necessary to make multiline tooltips in TooltipWnd work (at least, when hosted in a
		// Wnd-based window). See http://msdn.microsoft.com/en-us/library/ms906591.aspx
		LPNMHDR nm = (LPNMHDR)lp;
		if(nm->code==TTN_GETDISPINFO) {
			SendMessage(nm->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
			return 0;
		}
	}

	return DefWindowProc(_wnd, msg, wp, lp);
}

void Wnd::OnCopy() {
}

void Wnd::OnPaste() {
}

void Wnd::OnCut() {
}

void Wnd::TranslateKeyCodes(int vk, Key& key, wchar_t& ch) {
	ch = L'\0';

	switch(vk) {
		case VK_LEFT:
			key = KeyLeft;
			break;

		case VK_RIGHT:
			key = KeyRight;
			break;

		case VK_DOWN:
			key = KeyDown;
			break;

		case VK_UP:
			key = KeyUp;
			break;

		case VK_NEXT:
			key = KeyPageDown;
			break;

		case VK_PRIOR:
			key = KeyPageUp;
			break;

		case VK_MENU:
			key = KeyAlt;
			break;

		case VK_INSERT:
			key = KeyInsert;
			break;

		case VK_DELETE:
			key = KeyDelete;
			break;

		default:
			key = KeyCharacter;
			ch = (wchar_t)vk;
	}
}

// Default message handlers
void Wnd::OnSize(const Area& newSize) {
}

void Wnd::OnTimer(unsigned int id) {
	Repaint();
}

void Wnd::OnScroll(ScrollDirection dir) {
}

ref<Icon> Wnd::GetTabIcon() const {
	return null;
}

/* Property edit window */
LRESULT CALLBACK PropertyEditWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_KEYDOWN) {
		HWND first = GetWindow(wnd, GW_HWNDFIRST);
		HWND last = GetWindow(wnd, GW_HWNDLAST);

		if(wp==VK_DOWN || wp==VK_UP || wp==VK_TAB) {
			HWND root = GetAncestor(wnd, GA_ROOT);
			SetFocus(GetNextDlgTabItem(root, wnd, wp==VK_UP));
		}

		RECT rc;
		GetClientRect(wnd, &rc);
		InvalidateRect(wnd, &rc, FALSE);
	}
	WNDCLASS wc;
	GetClassInfo(0, L"EDIT", &wc);
	return CallWindowProc(wc.lpfnWndProc, wnd, msg, wp, lp);
}

std::wstring Wnd::GetText() {
	int n = GetWindowTextLength(_wnd);
	wchar_t* buffer = new wchar_t[n+2];
	GetWindowText(_wnd, buffer, n+1);
	std::wstring text = buffer;
	delete[] buffer;
	return text;
}

void Wnd::SetText(std::wstring text) {
	SetWindowText(_wnd, text.c_str());
}

// TODO: what to do on a DPI change? Resize all windows?
void Wnd::SetSize(Pixels w, Pixels h) {
	ref<Theme> theme = ThemeManager::GetTheme();
	SetWindowPos(_wnd, 0L, 0, 0, int(w*theme->GetDPIScaleFactor()), int(h*theme->GetDPIScaleFactor()), SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
}

Area Wnd::GetClientArea() const {
	RECT r;
	GetClientRect(_wnd, &r);
	ref<Theme> theme = ThemeManager::GetTheme();
	float df = theme->GetDPIScaleFactor();
	
	// Scale back to logical pixels
	Area area(r);
	area.MultiplyCeil(1/df, 1/df);
	return area;
}

Area Wnd::GetWindowArea() {
	RECT r;
	GetWindowRect(_wnd, &r);
	return Area(r);
}

void Wnd::SetWantMouseLeave(bool t) {
	_wantsMouseLeave = t;
	if(t) {
		TRACKMOUSEEVENT data;
		data.cbSize = sizeof(TRACKMOUSEEVENT);
		data.dwFlags = TME_LEAVE;
		data.hwndTrack = _wnd;
		data.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&data);
	}
}

bool Wnd::GetWantMouseLeave() const {
	return _wantsMouseLeave;
}

std::wstring Wnd::GetTabTitle() const {
	return L"";
}

void Wnd::Fill(LayoutFlags flags, Area& rect, bool direct) {
	switch(flags) {
		case LayoutFill:
		case LayoutTop:
		case LayoutBottom:
		case LayoutRight:
		case LayoutLeft:
			if(direct) {
				Move(rect.GetLeft(), rect.GetTop(), rect.GetWidth(), rect.GetHeight());
			}
			rect.SetWidth(0);
			rect.SetHeight(0);
			break;
		case LayoutHide:
		default:
			Move(0,0,0,0);
	}	
}

void Wnd::Fill() {
	RECT rc;
	HWND parent = ::GetParent(_wnd);
	GetClientRect(parent, &rc);
	Move(rc.left, rc.top, rc.right, rc.bottom);
}

void Wnd::OnActivate(bool a) {
}

void Wnd::OnFocus(bool a) {
}

void Wnd::OnMouse(MouseEvent me, Pixels x, Pixels y) {
}

/* Displays */
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData) {
	Displays* dp = (Displays*)dwData;
	if(dp!=0) {
		dp->AddDisplay(hMonitor);
	}
	return TRUE;
}

Displays::Displays() {
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)this);
}

Displays::~Displays() {
}

void Displays::AddDisplay(HMONITOR hm) {
	_displays.push_back(hm);
}

RECT Displays::GetDisplayRectangle(int idx) {
	if(int(_displays.size())>idx) {
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(_displays.at(idx), &mi);
		return mi.rcWork;
	}
	RECT r = {0,0,0,0};
	return r;
}

/** TopWnd **/
TopWnd::TopWnd(const wchar_t* title, HWND parent, const wchar_t* className,  bool usedb, int ex): Wnd(title, parent, className, usedb, ex), _quitOnClose(false), _oldStyle(0), _oldStyleEx(0), _fullScreen(false) {
}

TopWnd::~TopWnd() {
}

bool TopWnd::IsFullScreen() {
	return _fullScreen;
}

void TopWnd::OnSize(const Area& ns) {
	ref<Settings> st = GetSettings();
	Area rc = GetClientArea();
	if(st) {
		st->SetValue(L"width", Stringify(rc.GetWidth()));
		st->SetValue(L"height", Stringify(rc.GetHeight()));
	}

	Wnd::OnSize(ns);
} 

void TopWnd::OnSettingsChanged() {
	ref<Settings> st = GetSettings();

	Pixels w = StringTo<Pixels>(st->GetValue(L"width", L"0"),0);
	Pixels h = StringTo<Pixels>(st->GetValue(L"height", L"0"),0);
	int x = StringTo<int>(st->GetValue(L"x", L"0"), 0);
	int y = StringTo<int>(st->GetValue(L"y", L"0"), 0);

	// Set window position (but only if x>0 and y>0)
	if(x>=0 && y>=0) {
		SetWindowPos(GetWindow(), 0L, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
	}

	// Set window size (but only if w>0 and h>0)
	if(w>0 && h>0) {
		ref<Theme> theme = ThemeManager::GetTheme();
		SetWindowPos(GetWindow(), 0L, 0, 0l, long(w*theme->GetDPIScaleFactor()), long(h*theme->GetDPIScaleFactor()), SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
	}
}

void TopWnd::SetQuitOnClose(bool t) {
	_quitOnClose = t;
}

LRESULT TopWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CLOSE && _quitOnClose) {
		DestroyWindow(GetWindow());
		PostQuitMessage(0);
	}
	else if(msg==WM_MOVE) {
		ref<Settings> st = GetSettings();
		if(st) {
			RECT rc;
			GetWindowRect(GetWindow(), &rc);

			st->SetValue(L"x", Stringify(rc.left));
			st->SetValue(L"y", Stringify(rc.top));
		}
	}
	else if(msg==WM_GETMINMAXINFO) {
		MINMAXINFO* mm = (MINMAXINFO*)lp;
		Pixels w = 0;
		Pixels h = 0;
		GetMinimumSize(w,h);

		if(w!=0 && h!=0) {
			ref<Theme> theme = ThemeManager::GetTheme();
			mm->ptMinTrackSize.x = (long)(w * theme->GetDPIScaleFactor());
			mm->ptMinTrackSize.y = (long)(h * theme->GetDPIScaleFactor());
			return 0;
		}
	}

	return Wnd::Message(msg,wp,lp);
}

void TopWnd::GetMinimumSize(Pixels& w, Pixels& h) {
}

void TopWnd::SetFullScreen(bool fs) {
	if(fs==_fullScreen) return; //already in the desired mode
	HWND wnd = GetWindow();
	RECT rect;
	GetWindowRect(wnd, &rect);
	HMONITOR mon = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);

	MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(mon, &mi);

	if(fs && !_fullScreen) {
		_oldStyle = GetWindowLong(wnd, GWL_STYLE);
		_oldStyleEx = GetWindowLong(wnd, GWL_EXSTYLE);
		SetWindowLong(wnd, GWL_STYLE, (_oldStyle & (~WS_OVERLAPPEDWINDOW)) | (WS_VISIBLE|WS_POPUP));
		SetWindowPos(wnd,0,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,SWP_NOZORDER);
	}
	else if(_fullScreen) {
		if(_oldStyle!=0) {
			SetWindowLong(wnd, GWL_STYLE, _oldStyle);
			SetWindowLong(wnd, GWL_EXSTYLE, _oldStyleEx);
		}
		SetWindowPos(wnd,0,0,0,800,600,SWP_NOZORDER);
		UpdateWindow(wnd);
	}

	CloseHandle(mon);
	_fullScreen = fs;
}

void TopWnd::SetFullScreen(bool fs, int d) {
	HWND wnd = GetWindow();
	if(fs) {
		if(!_fullScreen) {
			_oldStyle = GetWindowLong(wnd, GWL_STYLE);
			_oldStyleEx = GetWindowLong(wnd, GWL_EXSTYLE);
			SetWindowLong(wnd, GWL_STYLE, (_oldStyle & (~WS_OVERLAPPEDWINDOW)) | (WS_VISIBLE|WS_POPUP));
		}

		Displays displays;
		RECT r = displays.GetDisplayRectangle(d);
		SetWindowPos(wnd, 0L, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOZORDER);
	}
	else if(_fullScreen) {
		if(_oldStyle!=0) {
			SetWindowLong(wnd, GWL_STYLE, _oldStyle);
			SetWindowLong(wnd, GWL_EXSTYLE, _oldStyleEx);
		}
		SetWindowPos(wnd,0,0,0,800,600,SWP_NOZORDER);
		UpdateWindow(wnd);
	}
	_fullScreen = fs;
}

/** Element **/
Element::Element() {
}

Element::~Element() {
}

Area Element::GetClientArea() const {
	return _client;
}

void Element::Fill(LayoutFlags flags, Area& rect, bool direct) {
	switch(flags) {
		case LayoutFill:
		case LayoutTop:
		case LayoutBottom:
		case LayoutRight:
		case LayoutLeft:
			if(direct) {
				Move(rect.GetLeft(), rect.GetTop(), rect.GetWidth(), rect.GetHeight());
			}
			rect.SetWidth(0);
			rect.SetHeight(0);
			break;
		case LayoutHide:
		default:
			Move(0,0,0,0);
	}
}

void Element::SetSize(Pixels w, Pixels h) {
	_client.SetHeight(h);
	_client.SetWidth(w);
	OnSize.Fire(ref<Object>(this), SizeNotification());
}

void Element::Move(Pixels x, Pixels y, Pixels w, Pixels h) {
	_client = Area(x,y,w,h);
	OnSize.Fire(ref<Object>(this), SizeNotification());
}

void Element::Update() {
	OnUpdate.Fire(ref<Object>(this), UpdateNotification());
}

void Element::Show(bool t) {
	_shown = t;
	OnShow.Fire(ref<Object>(this), ShowNotification(t));
}

bool Element::IsShown() const {
	return _shown;
}

Element::ShowNotification::ShowNotification(bool shown): _shown(shown) {
}