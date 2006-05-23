#include "../include/tjshared.h"
#include <commctrl.h>

using namespace Gdiplus;

bool Wnd::_classesRegistered = false;
LRESULT CALLBACK PropertyEditWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK PropertyEditNumericWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK PropertyLabelWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

/* GDI+ Init */
GraphicsInit::GraphicsInit() {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	InitCommonControls();
}

GraphicsInit::~GraphicsInit() {
	
}

void RecursivePaintChildren(HWND toplevel, HWND wp, Graphics& g) {
	if(wp==0) return;

	int style = GetWindowLong(wp, GWL_STYLE);
	if((style&WS_VISIBLE)==0) return;

	RECT tr;
	GetWindowRect(toplevel, &tr);
	RECT wr;
	GetWindowRect(wp, &wr);

	int xo = wr.left - tr.left;
	int yo = wr.top - tr.top;
	g.TranslateTransform(float(xo), float(yo));
 
	if(toplevel!=wp) {
		Wnd* wnd = reinterpret_cast<Wnd*>((long long)GetWindowLong(wp, GWL_USERDATA));
		if(wnd!=0) {
			wnd->Paint(g);
		}
		else {
			SendMessage(wp, WM_PRINTCLIENT, (WPARAM)g.GetHDC(), PRF_CLIENT|PRF_CHILDREN);
		}
	}

	g.TranslateTransform(-float(xo), -float(yo));
	HWND current = GetWindow(wp, GW_CHILD);
	if(current==0) return;
	current = GetWindow(current, GW_HWNDLAST);
	while(current!=0) {
		RecursivePaintChildren(toplevel, current, g);
		current = GetWindow(current, GW_HWNDPREV);
	}
}

BOOL ChildEnumeratorProc(HWND wnd, LPARAM lp);

class ChildEnumerator {
	public:
		ChildEnumerator(HWND parent, bool recursive=false);
		void Add(HWND wnd);
	
		std::vector<Wnd*> _children;
		bool _recursive;
		HWND _for;
};

BOOL ChildEnumeratorProc(HWND wnd, LPARAM lp) {
	ChildEnumerator* cp = reinterpret_cast<ChildEnumerator*>((long long)lp);
	if(cp!=0) {
		cp->Add(wnd);
	}
	return TRUE;
}

ChildEnumerator::ChildEnumerator(HWND parent, bool recursive): _for(parent) {
	_recursive = recursive;
	EnumChildWindows(parent,(WNDENUMPROC)ChildEnumeratorProc, (LPARAM)(long long)this);
}

void ChildEnumerator::Add(HWND wnd) {
	Wnd* cw = reinterpret_cast<Wnd*>((long long)GetWindowLong(wnd, GWL_USERDATA));
	if(cw!=0) {
		if(_recursive) {
			_children.push_back(cw);
			ChildEnumerator enu(wnd, true);
			std::vector<Wnd*>::iterator it = enu._children.begin();
			while(it!=enu._children.end()) {
				_children.push_back(*it);
				it++;
			}
		}
		else {
			if(cw->IsSplitter()) {
				SplitterWnd* sw = dynamic_cast<SplitterWnd*>(cw);
				if(sw) {
					_children.push_back(sw->_a.GetPointer()); // TODO: All GetPointers here are DANGEROUS
					if(sw->_a->IsSplitter()) {
						SplitterWnd* first = dynamic_cast<SplitterWnd*>(sw->_a.GetPointer());
						_children.push_back(first->_a.GetPointer());
						_children.push_back(first->_b.GetPointer());
					}

					_children.push_back(sw->_b.GetPointer());
					if(sw->_b->IsSplitter()) {
						SplitterWnd* first = dynamic_cast<SplitterWnd*>(sw->_b.GetPointer());
						_children.push_back(first->_a.GetPointer());
						_children.push_back(first->_b.GetPointer());
					}
				}
			}
			else {
				if(GetParent(wnd)!=_for) return;
				_children.push_back(cw);
			}
		}
	}
}

Wnd::Wnd(const wchar_t* title, HWND parent, const wchar_t* className, bool usedb) {
	RegisterClasses();
	_quitOnClose = false;
	_horizontalPos = 0;
	_verticalPos = 0;
	_horizontalPageSize = 1;
	_verticalPageSize = 1;
	_inHotkeyMode = false;
	_eatHotkeys = false;
	_fullScreen = false;
	_buffer = 0;
	_doubleBuffered = usedb;

	_wnd = CreateWindowEx(0L, className, title, WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, (HMENU)0, GetModuleHandle(NULL), 0);
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

void Wnd::EnterHotkeyMode() {
	_inHotkeyMode = true;
	SetFocus(_wnd);
	_inHotkeyMode = true;
	
	RECT rc;
	GetClientRect(_wnd, &rc);
	RedrawWindow(_wnd, &rc, 0, RDW_ALLCHILDREN|RDW_INTERNALPAINT);
	InvalidateRect(_wnd, &rc, FALSE);
	Repaint();
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

bool Wnd::IsShown() {
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
	if(msg==WM_CREATE) return 1;
	
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
	//wc.hIcon = LoadIcon(wc.hInstance, 0);
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = TJ_DEFAULT_CLASS_NAME;
	wc.style = CS_HREDRAW|CS_DBLCLKS;
	
	if(!RegisterClassEx(&wc)) {
		Throw(L"Could not register class",ExceptionTypeError);
	}
	
	wc.lpszClassName = TJ_DEFAULT_NDBL_CLASS_NAME;
	wc.style = CS_HREDRAW /* |CS_VREDRAW */;

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

bool Wnd::IsInHotkeyMode() {
	return _inHotkeyMode;
}

void Wnd::LeaveHotkeyMode(wchar_t key) {
	ReplyMessage(0);

	if(IsInHotkeyMode()) {
		_inHotkeyMode = false;
		
		if(key!=L'\0') {
			ChildEnumerator en(_wnd);
	
			bool found = false;
			std::vector<Wnd*>::iterator it = en._children.begin();
			while(it!=en._children.end()) {
				Wnd* w = *it;
				if(w->GetPreferredHotkey()==key) {
					w->EnterHotkeyMode();
					found = true;
					break;
				}
				it++;
			}

			if(!found) {
				MessageBeep(MB_ICONASTERISK);
			}
		}
	}
	
	//Application::Instance()->FullRepaint();
	Update();
}

bool Wnd::IsMouseOver() {
	POINT cursorPosition;
	GetCursorPos(&cursorPosition);

	RECT rc;
	GetWindowRect(_wnd, &rc);
	
	return (cursorPosition.x > rc.left && cursorPosition.y > rc.top && cursorPosition.x < rc.right && cursorPosition.y < rc.bottom);
}

LRESULT Wnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CLOSE) {
		Show(false);
		if(_quitOnClose) DestroyWindow(_wnd);
		return 0;
	}
	else if(msg==WM_LBUTTONDOWN) {
		SetFocus(_wnd);
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

		Repaint();
		return 0;
	}
	else if(msg==WM_LBUTTONDOWN||msg==WM_LBUTTONUP) {
		//LeaveHotkeyMode();
	}
	else if(msg==WM_DESTROY) {
		if(_quitOnClose) PostQuitMessage(0);
		return 0;
	}
	else if(msg==WM_KEYDOWN) {
		if(_eatHotkeys) {
			/*ref<View> vw = Application::Instance()->GetView();
			if(vw) {
				if(wp==VK_SPACE) {
					vw->Command(ID_JUMP);
				}
				else if(wp==L'C') {
					vw->Command(ID_CREATE_CUE);
				}
			}*/
		}

		if(IsInHotkeyMode()) {
			LeaveHotkeyMode((wchar_t)wp);
		}
	}
	else if(msg==WM_SETFOCUS) {
		return 0;
	}
	else if(msg==WM_KILLFOCUS) {
		if(IsInHotkeyMode()) {
			LeaveHotkeyMode(L'\0');
		}
	}

	return DefWindowProc(_wnd, msg, wp, lp);
}

void Wnd::DrawHotkey(Graphics* g, const wchar_t* str, int x, int y) {
	assert(str!=0);	
	ref<Theme> theme = ThemeManager::GetTheme();

	RectF blockrc(float(x-8), float(y-8), 16.0f, 16.0f);
	SolidBrush br(theme->GetTextColor());
	SolidBrush line(theme->GetLineColor());
	LinearGradientBrush lbr(Gdiplus::Point(x, y-8), Gdiplus::Point(x,y+8), theme->GetActiveStartColor(),theme->GetActiveEndColor());
	g->FillRectangle(&lbr,blockrc);
	StringFormat sf;
	sf.SetAlignment(StringAlignmentCenter);
	g->DrawString(str,(int)wcslen(str),theme->GetGUIFontBold(),blockrc,&sf,&br);
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

void Wnd::SetEatHotkeys(bool e) {
	_eatHotkeys = e;
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
	if(msg==WM_PAINT) {
		PAINTSTRUCT ps;
		BeginPaint(_wnd, &ps);
		Graphics g(ps.hdc);

		RECT r;
		GetClientRect(_wnd,&r);

		SolidBrush br(Color(_r, _g, _b));
		g.FillRectangle(&br, 0,0,r.right-r.left, r.bottom-r.top);

		EndPaint(_wnd, &ps);
		return 0;
	}
	return Wnd::Message(msg,wp,lp);
}

void Wnd::Move(int x, int y, int w, int h) {
	//MoveWindow(_wnd,x,y,w,h,FALSE);
	SetWindowPos(_wnd, 0L, x, y, w, h, SWP_NOZORDER|SWP_NOREDRAW|SWP_NOSENDCHANGING);
	//Layout();
}

LRESULT Wnd::PreMessage(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_PAINT) {
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

		Wnd* parent = GetParent();
		if(parent!=0) {
			if(parent->IsInHotkeyMode()) { 
				wchar_t hotkey = GetPreferredHotkey();
				if(hotkey!=L'\0') {
					RECT rc;
					GetClientRect(_wnd, &rc);
					//Graphics org(ps.hdc);
					std::wostringstream os;
					os << hotkey;
					std::wstring hk = os.str();
					DrawHotkey(&org, hk.c_str(), (rc.right-rc.left)/2, (rc.bottom-rc.top)/2);
				}
			}
		}

		EndPaint(_wnd, &ps);
		return 0;
	}
	else if(msg==WM_TJ_PRINT) {
		Graphics* g = (Graphics*)lp;

		//if((lp&PRF_CHILDREN)!=0) {
			RecursivePaintChildren(GetWindow(), GetWindow(), *g);
		//}

		return 0;
	}
	else {
		return Message(msg, wp, lp);
	}
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
		else if(wp==VK_RETURN) {
			return 0; // do not play
		}
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