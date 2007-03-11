#include "../../include/tjshared.h"
#include <windowsx.h>
using namespace tj::shared;
using namespace Gdiplus;

/* PropertyGridWnd implementation */
PropertyGridWnd::PropertyGridWnd(HWND parent): ChildWnd(TL(properties), parent) {
	ClearThemeCache();
	_nameWidth = 100;
	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	SetStyleEx(WS_EX_CONTROLPARENT);
	_editBackground = 0;
	_editFont = 0;
	_isDraggingSplitter = false;
	_path = GC::Hold(new PathWnd(GetWindow(), this));
	_path->Show(true);
	Layout();
	Show(true);
}

PropertyGridWnd::~PropertyGridWnd() {
	if(_editBackground!=0) DeleteObject(_editBackground);
	_editBackground = 0;

	if(_editFont!=0) DeleteObject(_editFont);
	_editFont = 0;
}

void PropertyGridWnd::SetNameWidth(int w) {
	_nameWidth = w;
	Repaint();
}

void PropertyGridWnd::ClearThemeCache() {
	if(_editBackground!=0) DeleteObject(_editBackground);
	Color back = ThemeManager::GetTheme()->GetPropertyBackgroundColor();
	_editBackground = CreateSolidBrush(RGB(back.GetRed(),back.GetGreen(),back.GetBlue()));

	if(_editFont!=0) DeleteObject(_editFont);
	_editFont = CreateFont(-10, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
}

void PropertyGridWnd::Paint(Graphics& g) {
	RECT r;
	GetClientRect(GetWindow(), &r);
	ref<Theme> theme = ThemeManager::GetTheme();

	SolidBrush br(theme->GetPropertyBackgroundColor());
	g.FillRectangle(&br,-1,-1,r.right-r.left+1,r.bottom-r.top+1);

	int cH = KPathHeight-GetVerticalPos();
	int hI = 0;
	std::vector< ref<Property> >::iterator it = _properties.begin();

	// TODO: cache theme colors over here
	float stringLeft = 5.0f;
	while(it!=_properties.end()) {
		ref<Property> p = *it;
		if(!p) continue;

		p->Update();
		bool bold = p.IsCastableTo<PropertySeparator>();

		if(GetFocus()==p->GetWindow() || bold) {
			LinearGradientBrush gbr(Gdiplus::Point(0, cH), Gdiplus::Point(0, cH+p->GetHeight()+10), theme->GetActiveStartColor(), theme->GetActiveEndColor());
			g.FillRectangle(&gbr, Rect(1, cH+1, r.right-r.left-2, p->GetHeight()+6-2));
		}

		if(bold) {
			SolidBrush dbr(theme->GetDisabledOverlayColor());
			g.FillRectangle(&dbr, Rect(1, cH+1, r.right-r.left-2, p->GetHeight()+6-2));
		}

		std::wstring ws = p->GetName();
		SolidBrush tb(theme->GetTextColor());
		
		g.DrawString(ws.c_str(), (int)ws.length(),bold?theme->GetGUIFontBold():theme->GetGUIFont(), PointF(stringLeft, float(cH+5)), &tb);
		
		cH += p->GetHeight() + 6;

		it++;
		hI++;
	}
}

void PropertyGridWnd::OnScroll(ScrollDirection dir) {
	Layout();
	Repaint();
}

LRESULT PropertyGridWnd::Message(UINT msg, WPARAM wParam, LPARAM lParam) {
	if(msg==WM_CLOSE) {
		ShowWindow(GetWindow(),SW_HIDE);
		return 1;
	}
	else if(msg==WM_CONTEXTMENU) {
		return 0;
	}
	else if(msg==WM_COMMAND) {
		if(HIWORD(wParam)==EN_UPDATE||HIWORD(wParam)==BN_CLICKED||HIWORD(wParam)==CBN_SELCHANGE) {
			HWND he = (HWND)lParam;

			std::vector< ref<Property> >::iterator it = _properties.begin();
			while(it!=_properties.end()) {
				ref<Property> prw = (*it);
				assert(prw!=0);

				if(prw->GetWindow()==he) {
					prw->Changed();
					break;
				}
				it++;
			}
		}
		else if(HIWORD(wParam)==EN_SETFOCUS) {
			Layout();
		}
		
		if(HIWORD(wParam)==EN_CHANGE /*||HIWORD(wParam)==EN_UPDATE */ || HIWORD(wParam)==BN_CLICKED||HIWORD(wParam)==CBN_SELCHANGE) {
			Wnd* root = GetRootWindow();
			if(root!=0) {
				root->Update();
			}
			
		}
	}
	else if(msg==WM_CTLCOLOREDIT) {
		wchar_t className[100];
		GetClassName((HWND)lParam, className, 100-1);
		if(_wcsicmp(className, L"COMBOBOX")!=0) {
			SetBkMode((HDC)wParam, TRANSPARENT);
			Color text = ThemeManager::GetTheme()->GetTextColor();
			SetTextColor((HDC)wParam, RGB(text.GetRed(),text.GetGreen(),text.GetBlue()));
			return (LRESULT)(HBRUSH)_editBackground;
		}
	}
	else if(msg==WM_CTLCOLORBTN) {
		SetBkMode((HDC)wParam, TRANSPARENT);
		return (LRESULT)(HBRUSH)_editBackground;
	}
	else if(msg==WM_CTLCOLORSTATIC) {
		SetBkMode((HDC)wParam, TRANSPARENT);
		Color text = ThemeManager::GetTheme()->GetTextColor();
		//Color text(255,0,0);
		SetTextColor((HDC)wParam, RGB(text.GetRed(),text.GetGreen(),text.GetBlue()));
		return (LRESULT)(HBRUSH)_editBackground;
	}
	else if(msg==WM_ACTIVATE) {
		OnSize(GetClientArea());
		Layout();
		return 0;
	}
	else if(msg==WM_LBUTTONUP) {
		_isDraggingSplitter = false;
		Layout();
		ReleaseCapture();
		return 0;
	}
	else if(msg==WM_LBUTTONDOWN) {
		int x = GET_X_LPARAM(lParam);
		if(x>(_nameWidth-5) && x<(_nameWidth+5)) {
			_isDraggingSplitter = true;
			SetCapture(GetWindow());
		}
	}
	else if(msg==WM_MOUSEMOVE) {
		int x = GET_X_LPARAM(lParam);

		if(_isDraggingSplitter) {
			_nameWidth = x;
			Layout();
		}

		if(x>(_nameWidth-5) && x<(_nameWidth+5)) {
			SetCursor(LoadCursor(0, IDC_SIZEWE));
		}
		else {
			SetCursor(LoadCursor(0, IDC_ARROW));
		}
	}

	return ChildWnd::Message(msg,wParam, lParam);
}

void PropertyGridWnd::Layout() {
	Area rect = GetClientArea();

	if(_path) {
		_path->Move(0,0,rect.GetWidth(), KPathHeight);
		SetWindowPos(_path->GetWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}

	int cH = KPathHeight - GetVerticalPos();
	std::vector<ref<Property> >::iterator it = _properties.begin();

	while(it!=_properties.end()) {
		ref<Property> pr = *it;
		if(pr) {
			HWND vw = pr->GetWindow();
			int h = pr->GetHeight();
			SendMessage(vw, WM_SETFONT, (WPARAM)(HFONT)_editFont, FALSE);
			LONG style = GetWindowLong(vw, GWL_STYLE);
			style |= WS_CLIPSIBLINGS;
			SetWindowLong(vw, GWL_STYLE, style);
			SetWindowPos(vw, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			SetWindowPos(vw, 0,  _nameWidth, cH+3,  rect.GetWidth()-_nameWidth-3, h, SWP_NOZORDER);
			ShowWindow(vw, SW_SHOW);
			cH += h + 6;
		}
		it++;
	}
	Repaint();
}

void PropertyGridWnd::OnSize(const Area& ns) {
	Area rect = GetClientArea();
	// Get total property height
	int totalHeight = 0;
	std::vector< ref<Property> >::iterator it = _properties.begin();
	while(it!=_properties.end()) {
		ref<Property> pr = *it;
		if(pr) {
			totalHeight += pr->GetHeight() + 6;
		}
		it++;
	}

	if(totalHeight>rect.GetHeight()) {
		SetVerticallyScrollable(true);
		SetVerticalScrollInfo(Range<unsigned int>(0, totalHeight+KPathHeight), rect.GetHeight());
	}
	else {
		SetVerticallyScrollable(false);
		SetVerticalPos(0);
	}
	Layout();
	Repaint();
}

void PropertyGridWnd::Update() {
	std::vector< ref<Property> >::iterator it = _properties.begin();
	while(it!=_properties.end()) {
		ref<Property> pr = (*it);
		pr->Update();
		it++;
	}
	_path->Update();
	Repaint();
}

void PropertyGridWnd::Clear() {
	_properties.clear();
	Layout();
}

void PropertyGridWnd::Inspect(Inspectable* isp, ref<Path> p) {
	_properties.clear();

	if(isp==0) {	
		ClearThemeCache();
		Layout();
		return;
	}

	ref< std::vector< ref<Property> > > props = isp->GetProperties();
	if(!props) {
		ClearThemeCache();
		Layout();
		return;
	}

	_properties.clear();
	std::vector< ref<Property> >::iterator it = props->begin();
	HWND first = 0;
	while(it!=props->end()) {
		ref<Property> pr = *it;
		if(pr!=0) {
			HWND f = pr->Create(GetWindow());
			if(first==0) {
				first = f;
			}

			_properties.push_back(pr);
		}
		it++;
	}

	_path->SetPath(p);
	SetFocus(first);
	ClearThemeCache();
	OnSize(GetClientArea());
	Layout();
}

void PropertyGridWnd::Inspect(ref<Inspectable> isp, ref<Path> p) {
	if(!isp) {
		_properties.clear();
		return;
	}

	Inspectable* is = isp.GetPointer();
	if(is==0) {
		Throw(L"GetPointer==0!", ExceptionTypeError);
	}
	Inspect(is,p);
}