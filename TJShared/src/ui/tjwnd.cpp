#include "../../include/tjshared.h"
#include <commctrl.h>
#include <windowsx.h>
#define ISVKKEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000))

using namespace tj::shared;
using namespace Gdiplus;

bool Wnd::_classesRegistered = false;
LRESULT CALLBACK PropertyEditWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK PropertyEditNumericWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK PropertyEditTimeWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK PropertyLabelWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

/* GDI+ Init */
GraphicsInit::GraphicsInit() {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	INITCOMMONCONTROLSEX sex;
	sex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	sex.dwICC = ICC_STANDARD_CLASSES|ICC_TAB_CLASSES|ICC_PROGRESS_CLASS|ICC_UPDOWN_CLASS|ICC_USEREX_CLASSES|ICC_WIN95_CLASSES;
	InitCommonControlsEx(&sex);
}

GraphicsInit::~GraphicsInit() {
	
}

Wnd::Wnd(const wchar_t* title, HWND parent, const wchar_t* className, bool usedb, int exStyle) {
	RegisterClasses();
	_quitOnClose = false;
	_horizontalPos = 0;
	_verticalPos = 0;
	_horizontalPageSize = 1;
	_verticalPageSize = 1;
	_fullScreen = false;
	_buffer = 0;
	_doubleBuffered = usedb;

	_wnd = CreateWindowEx(exStyle, className, title, WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, (HMENU)0, GetModuleHandle(NULL), (void*)this);
	if(_wnd==0) Throw(L"Could not create window", ExceptionTypeError);

	SetWindowLong(_wnd, GWL_USERDATA, (LONG)(long long)this);

	SetParent(_wnd,parent);
	UpdateWindow(_wnd);
	_oldStyle = 0;
	_oldStyleEx = 0;
}

void Wnd::SetText(const wchar_t* t) {
	SetWindowText(_wnd, t);
}

bool Wnd::IsFullScreen() {
	return _fullScreen;
}

void Wnd::Update() {
}

void Wnd::Layout() {
}

void Wnd::SetFullScreen(bool fs) {
	if(fs==_fullScreen) return; //already in the desired mode
	RECT rect;
	GetWindowRect(_wnd,&rect);
	HMONITOR mon = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);

	MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(mon, &mi);

	/* TODO hier iets doen met schermgrootte of dualscreen troep */
	if(fs&&!_fullScreen) {
		_oldStyle = GetWindowLong(_wnd, GWL_STYLE);
		_oldStyleEx = GetWindowLong(_wnd, GWL_EXSTYLE);
		SetWindowLong(_wnd, GWL_STYLE, (_oldStyle & (~WS_OVERLAPPEDWINDOW)) | (WS_VISIBLE|WS_POPUP));
		SetWindowPos(_wnd,0,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,SWP_NOZORDER);
	}
	else if(_fullScreen) {
		if(_oldStyle!=0) {
			SetWindowLong(_wnd, GWL_STYLE, _oldStyle);
			SetWindowLong(_wnd, GWL_EXSTYLE, _oldStyleEx);
		}
		SetWindowPos(_wnd,0,0,0,800,600,SWP_NOZORDER);
		UpdateWindow(_wnd);
		Repaint();
	}

	CloseHandle(mon);
	_fullScreen = fs;
}

void Wnd::Focus() {
	::SetFocus(_wnd);
}

void Wnd::SetFullScreen(bool fs, int d) {
	if(fs) {
		if(!_fullScreen) {
			_oldStyle = GetWindowLong(_wnd, GWL_STYLE);
			_oldStyleEx = GetWindowLong(_wnd, GWL_EXSTYLE);
			SetWindowLong(_wnd, GWL_STYLE, (_oldStyle & (~WS_OVERLAPPEDWINDOW)) | (WS_VISIBLE|WS_POPUP));
		}

		Displays displays;
		RECT r = displays.GetDisplayRectangle(d);
		Log::Write(L"TJShared/UI", L"Set full screen: d="+Stringify(d)+L" Rect="+Stringify(r.left)+L","+Stringify(r.top)+L"x"+Stringify(r.right)+L","+Stringify(r.bottom));
		SetWindowPos(_wnd, 0L, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOZORDER);
	}
	else if(_fullScreen) {
		if(_oldStyle!=0) {
			SetWindowLong(_wnd, GWL_STYLE, _oldStyle);
			SetWindowLong(_wnd, GWL_EXSTYLE, _oldStyleEx);
		}
		SetWindowPos(_wnd,0,0,0,800,600,SWP_NOZORDER);
		UpdateWindow(_wnd);
		Repaint();
	}
	_fullScreen = fs;
}

bool Wnd::IsSplitter() {
	return false;
}

void Wnd::Show(bool t) {
	ShowWindow(_wnd, t?SW_SHOW:SW_HIDE);
	if(!t) {
		delete _buffer;
		_buffer = 0;
	}
}

bool Wnd::HasFocus() {
	return GetFocus()==_wnd;
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
	if(msg==WM_CREATE) {
		return 1;
	}
	else if(msg==WM_DESTROY) {
		return 1;
	}
	
	Wnd* dp = reinterpret_cast<Wnd*>((long long)GetWindowLong(wnd,GWL_USERDATA));

	if(dp!=0) {
		return dp->PreMessage(msg,wp,lp);
	}

	return DefWindowProc(wnd, msg, wp, lp);
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

	wc.lpszClassName = TJ_GL_CLASS_NAME;
	wc.style = CS_CLASSDC;
	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class", ExceptionTypeError);
	}
	
	wc.style = CS_HREDRAW|CS_DBLCLKS;
	wc.lpszClassName = TJ_DEFAULT_NDBL_CLASS_NAME;
	wc.style = CS_HREDRAW;

	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class",ExceptionTypeError);
	}

	GetClassInfoEx(0, L"EDIT", &wc);
	wc.lpszClassName = TJ_PROPERTY_EDIT_CLASS_NAME;
	wc.lpfnWndProc = PropertyEditWndProc;

	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class", ExceptionTypeError);
	}

	wc.lpszClassName = TJ_PROPERTY_EDIT_NUMERIC_CLASS_NAME;
	wc.lpfnWndProc = PropertyEditNumericWndProc;
	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class", ExceptionTypeError);
	}

	wc.lpszClassName = TJ_PROPERTY_EDIT_TIME_CLASS_NAME;
	wc.lpfnWndProc = PropertyEditTimeWndProc;
	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class", ExceptionTypeError);
	}

	wc.lpszClassName = TJ_PROPERTY_LABEL_CLASS_NAME;
	wc.lpfnWndProc = PropertyLabelWndProc;
	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class", ExceptionTypeError);
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

void Wnd::SetQuitOnClose(bool q) {
	_quitOnClose = q;
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

unsigned int Wnd::GetHorizontalPos() {
	return _horizontalPos;
}

unsigned int Wnd::GetVerticalPos() {
	return _verticalPos;
}

void Wnd::SetVerticalPos(unsigned int p) {
	SCROLLINFO si;
	memset(&si,0,sizeof(SCROLLINFO));
	si.fMask = SIF_POS;
	si.nPos = p;
	SetScrollInfo(_wnd, SB_VERT, &si, TRUE);
	_verticalPos = p;
}

void Wnd::SetHorizontalPos(unsigned int p) {
	SCROLLINFO si;
	memset(&si,0,sizeof(SCROLLINFO));
	si.fMask = SIF_POS;
	si.nPos = p;
	SetScrollInfo(_wnd, SB_HORZ, &si, TRUE);
	_horizontalPos = p;
}

void Wnd::SetHorizontalScrollInfo(Range<unsigned int> rng, unsigned int pageSize) {
	SCROLLINFO srl;
	memset(&srl,0,sizeof(SCROLLINFO));
	srl.cbSize = sizeof(SCROLLINFO);
	srl.fMask = SIF_RANGE|SIF_PAGE;
	srl.nMax = rng.End();
	srl.nMin = rng.Start();
	_horizontalPageSize = pageSize;
	srl.nPage = pageSize;
	
	SetScrollInfo(_wnd, SB_HORZ,&srl,TRUE);
}

void Wnd::SetVerticalScrollInfo(Range<unsigned int> rng, unsigned int pageSize) {
	SCROLLINFO srl;
	memset(&srl,0,sizeof(SCROLLINFO));
	srl.cbSize = sizeof(SCROLLINFO);
	srl.fMask = SIF_RANGE|SIF_PAGE;
	srl.nMax = rng.End();
	srl.nMin = rng.Start();
	srl.nPage = pageSize;

	_verticalPageSize = pageSize;
	SetScrollInfo(_wnd, SB_VERT,&srl,TRUE);
}

LRESULT ColorWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	return Wnd::Message(msg,wp,lp);
}

void ColorWnd::Paint(Gdiplus::Graphics& g) {
	SolidBrush br(Color(_r, _g, _b));
	g.FillRectangle(&br, GetClientArea());
}


void Wnd::Move(int x, int y, int w, int h) {
	//MoveWindow(_wnd,x,y,w,h,FALSE);
	SetWindowPos(_wnd, 0L, x, y, w, h, SWP_NOZORDER|SWP_NOREDRAW|SWP_NOSENDCHANGING);
	//Layout();
}

LRESULT Wnd::PreMessage(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_ERASEBKGND) {
		return 1;
	}
	else if(msg==WM_PAINT) {
		int style = GetWindowLong(_wnd, GWL_STYLE);
		if((style&WS_VISIBLE)==0) {
			return 0;
		}

		PAINTSTRUCT ps;
		BeginPaint(_wnd, &ps);
		Graphics org(ps.hdc);

		if(!_doubleBuffered) {
			Paint(org);
		}
		else {
			RECT cw;
			GetClientRect(_wnd, &cw);
			if(_buffer==0 || int(_buffer->GetWidth()) < int(cw.right-cw.left) || int(_buffer->GetHeight()) < int(cw.bottom-cw.top)) {
				delete _buffer;
				_buffer = 0;
				_buffer = new Bitmap(cw.right-cw.left+100, cw.bottom-cw.top+100, &org); // +100 for buffer
			}

			{
				Graphics buffered(_buffer);
				Paint(buffered);
			}
			org.DrawImage(_buffer,0,0);
		}

		EndPaint(_wnd, &ps);
		return 0;
	}	
	else {
		return Message(msg, wp, lp);
	}
}

LRESULT Wnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CLOSE) {
		Show(false);
		if(_quitOnClose) {
			DestroyWindow(_wnd);
			PostQuitMessage(0);
		}
		return 0;
	}
	else if(msg==WM_LBUTTONDOWN) {
		SetFocus(_wnd);
	}
	else if(msg==WM_ACTIVATE) {
		OnActivate(LOWORD(wp)!=WA_INACTIVE);
		return 0;
	}
	else if(msg==WM_SIZE) {
		Area size = GetClientArea();
		//size.SetWidth(LOWORD(lp));
		//size.SetHeight(HIWORD(lp));
		if(!(size.GetWidth()==0 || size.GetHeight()==0)) {
			/* window is hidden, don't bother changing layout
			   Some windows add scrollbars when the area is very small (0,0)
			   and this prevents them from doing so */
			OnSize(size);
		}
		
		return 0;
	}
	else if(msg==WM_HSCROLL) {
		SCROLLINFO inf;
		GetScrollInfo(_wnd, SB_HORZ, &inf);
		if(LOWORD(wp)==SB_THUMBTRACK||LOWORD(wp)==SB_THUMBPOSITION) {
			// _horizontalPos = HIWORD(wp); // fails because our values are >65536
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_TRACKPOS;
 
            GetScrollInfo(_wnd, SB_HORZ, &si);
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
				_horizontalPos += 1;			
			}
			else if(LOWORD(wp)==SB_LINELEFT) {
				_horizontalPos -= 1;			
			}
			SetScrollPos(_wnd,SB_HORZ, _horizontalPos,TRUE);
		}

		OnScroll(ScrollDirectionHorizontal);
		Repaint();
		return 0;
	}
	else if(msg==WM_VSCROLL) {
		SCROLLINFO inf;
		GetScrollInfo(_wnd, SB_VERT, &inf);
		if(LOWORD(wp)==SB_THUMBTRACK||LOWORD(wp)==SB_THUMBPOSITION) {
			_verticalPos = HIWORD(wp);
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
				_verticalPos += 1;			
			}
			else if(LOWORD(wp)==SB_LINELEFT) {
				_verticalPos -= 1;			
			}
			SetScrollPos(_wnd,SB_VERT, _verticalPos,TRUE);
		}

		OnScroll(ScrollDirectionVertical);
		Repaint();
		return 0;
	}
	else if(msg==WM_SETFOCUS) {
		return 0;
	}
	else if(msg==WM_MOUSEMOVE) {
		SetWantMouseLeave(_wantsMouseLeave);
	}

	return DefWindowProc(_wnd, msg, wp, lp);
}


// Default message handlers
void Wnd::OnSize(const Area& newSize) {
}

void Wnd::OnScroll(ScrollDirection dir) {
}

/* Property edit window */
LRESULT CALLBACK PropertyEditWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_KEYDOWN) {
		HWND first = GetWindow(wnd, GW_HWNDFIRST);
		HWND last = GetWindow(wnd, GW_HWNDLAST);

		if(wp==VK_DOWN) {
			if(wnd==last) {
				SetFocus(first);
			}
			else {
				HWND next = GetWindow(wnd, GW_HWNDNEXT);
				SetFocus(next);
			}
		}
		else if(wp==VK_UP) {
			if(wnd==first) {
				SetFocus(last);
			}
			else {
				HWND next = GetWindow(wnd, GW_HWNDPREV);
				SetFocus(next);
			}
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

void Wnd::SetSize(int w, int h) {
	SetWindowPos(_wnd, 0L, 0, 0, w, h, SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
}

Area Wnd::GetClientArea() {
	RECT r;
	GetClientRect(_wnd, &r);
	return Area(r);
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

void Wnd::Fill(LayoutFlags flags, Area& rect) {
	switch(flags) {
		case LayoutFill:
		case LayoutTop:
		case LayoutBottom:
		case LayoutRight:
		case LayoutLeft:
			Move(rect.GetLeft(), rect.GetTop(), rect.GetWidth(), rect.GetHeight());
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

LRESULT CALLBACK PropertyEditNumericWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	static int begin_y = -1;
	static int begin_value = -1;

	if(msg==WM_KEYDOWN) {
		if(wp==VK_RIGHT||wp==VK_LEFT) {
			DWORD n = GetWindowTextLength(wnd);
			wchar_t* buf = new wchar_t[n+2];
			GetWindowText(wnd, buf, n+1);

			int data = _wtoi(buf);
			data = data + ((wp==VK_LEFT)?-1:1);
			
			std::wostringstream os;
			os << data;
			std::wstring getal = os.str();
			SetWindowText(wnd, getal.c_str());
			return 0;
		}
		if((wp>L'9' || wp < L'0')&&wp!=L'-' &&wp!=L':' && wp!=VK_DELETE && wp!=VK_HOME && wp!=VK_END && wp!=L'\b' && wp!=VK_UP && wp!=VK_DOWN) {
			return 0; // no alpha-numeric stuff in here please!
		}
	}
	else if(msg==WM_CHAR) {
		if((wp>L'9' || wp < L'0')&&wp!=L'-' && wp!=L':' && wp!=VK_DELETE && wp!=VK_HOME && wp!=VK_END && wp!=L'\b' && wp!=VK_UP && wp!=VK_DOWN) {
			return 0; // no alpha-numeric stuff in here please!
		}
	}
	else if(msg==WM_MBUTTONDOWN) {
		SetCapture(wnd);
		begin_y = GET_Y_LPARAM(lp);

		int n = GetWindowTextLength(wnd);
		wchar_t* buffer = new wchar_t[n+2];
		GetWindowText(wnd, buffer, n+1);
		std::wstring value = buffer;
		delete[] buffer;

		begin_value = StringTo<int>(value, -1);
	}
	else if(msg==WM_MBUTTONUP) {
		ReleaseCapture();
		begin_y = begin_value = -1;
	}
	else if(msg==WM_MOUSEMOVE && ISVKKEYDOWN(VK_MBUTTON)) {
		if(begin_y>=0) {
			int dy = begin_y - GET_Y_LPARAM(lp);

			if(begin_value!=-1) {
				int x = dy*dy;
				if(dy<0) x = -x;
				std::wstring newValue = Stringify(begin_value+x);
				SetWindowText(wnd, newValue.c_str());
			}
		}
	}
	return PropertyEditWndProc(wnd, msg, wp, lp);
}

LRESULT CALLBACK PropertyEditTimeWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	return PropertyEditWndProc(wnd, msg, wp, lp);
}

LRESULT CALLBACK PropertyLabelWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_PAINT) {
		PAINTSTRUCT ps;
		BeginPaint(wnd, &ps);
		Graphics g(ps.hdc);

		ref<Theme> theme = ThemeManager::GetTheme();
		RECT rc;
		GetClientRect(wnd, &rc);

		SolidBrush backBrush(theme->GetPropertyBackgroundColor());
		SolidBrush textBrush(theme->GetTextColor());

		int n = GetWindowTextLength(wnd);
		wchar_t* str = new wchar_t[n+2];
		GetWindowText(wnd, str, n+1);

		RectF rcf((float)rc.left, (float)rc.top, float(rc.right-rc.left), float(rc.bottom-rc.top));
		g.FillRectangle(&backBrush, rcf);
		rcf.Y += 3;
		g.DrawString(str,n, theme->GetGUIFont(),rcf, 0, &textBrush);
		delete[] str;

		EndPaint(wnd, &ps);
		return 0;
	}
	return DefWindowProc(wnd, msg, wp, lp);
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