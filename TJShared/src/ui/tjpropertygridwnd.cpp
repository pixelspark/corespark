#include "../../include/tjshared.h"
#include <windowsx.h>
#include <commctrl.h>
using namespace tj::shared;
using namespace Gdiplus;

/// TODO: Alles netjes Pixels maken ipv int (ook scrollbar), een GetPropertyAt(y=?) maken en de expand/collapse
/// code fixen (en misschien sommige properties die nog niet met Pixels werken even fixen).

/* PropertyGridWnd implementation */
PropertyGridWnd::PropertyGridWnd(bool withPath): ChildWnd(TL(properties)),
_expandIcon(L"icons/shared/expand.png"), _collapseIcon(L"icons/shared/collapse.png") {
	ClearThemeCache();
	_nameWidth = 100;
	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	SetStyleEx(WS_EX_CONTROLPARENT);
	SetVerticallyScrollable(true);
	_editBackground = 0;
	_editFont = 0;
	_showHints = true;
	_isDraggingSplitter = false;

	if(withPath) {
		_path = GC::Hold(new PathWnd(this));
		Add(_path);
	}
	Layout();
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

void PropertyGridWnd::SetShowHints(bool t) {
	_showHints = t;
	ref<Settings> st = GetSettings();
	if(st) {
		st->SetValue(L"hints.show", _showHints?L"yes":L"no");
	}
}

bool PropertyGridWnd::GetShowHints() const {
	return _showHints;
}

void PropertyGridWnd::OnSettingsChanged() {
	ref<Settings> st = GetSettings();
	_nameWidth = StringTo<int>(st->GetValue(L"names.width", Stringify(_nameWidth)), _nameWidth);

	_showHints = st->GetValue(L"hints.show", _showHints?L"yes":L"no")!=L"no";
	if(_nameWidth<KMinimumNameColumnWidth) _nameWidth = KMinimumNameColumnWidth;

	if(_path) {
		_path->SetSettings(st->GetNamespace(L"path"));
	}
}

void PropertyGridWnd::ClearThemeCache() {
	if(_editBackground!=0) DeleteObject(_editBackground);
	Color back = ThemeManager::GetTheme()->GetPropertyBackgroundColor();
	_editBackground = CreateSolidBrush(RGB(back.GetRed(),back.GetGreen(),back.GetBlue()));

	if(_editFont!=0) DeleteObject(_editFont);
	_editFont = CreateFont(-10, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
}

void PropertyGridWnd::Paint(Graphics& g, ref<Theme> theme) {
	Area r = GetClientArea();

	SolidBrush br(theme->GetPropertyBackgroundColor());
	g.FillRectangle(&br,-1,-1,r.GetWidth()+1,r.GetHeight()+1);

	Pixels cH = GetPathHeight()-GetVerticalPos();
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
			g.FillRectangle(&gbr, Rect(1, cH+1, r.GetWidth()-2, p->GetHeight()+6-2));
		}

		if(bold) {
			SolidBrush dbr(theme->GetDisabledOverlayColor());
			g.FillRectangle(&dbr, Rect(1, cH+1, r.GetWidth()-2, p->GetHeight()+6-2));
		}

		std::wstring ws = p->GetName();
		SolidBrush tb(theme->GetTextColor());
		
		g.DrawString(ws.c_str(), (int)ws.length(),bold?theme->GetGUIFontBold():theme->GetGUIFont(), PointF(stringLeft, float(cH+5)), &tb);
		
		if(p->IsExpandable()) {
			Area expander(_nameWidth-21, cH+3, 16, 16);
			if(p->IsExpanded()) {
				g.DrawImage(_collapseIcon, expander);
			}
			else {
				g.DrawImage(_expandIcon, expander);
			}
		}
		

		cH += p->GetHeight() + 6;

		++it;
		hI++;
	}
}

void PropertyGridWnd::OnScroll(ScrollDirection dir) {
	Layout();
	Repaint();
}

void PropertyGridWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLUp) {
		if(_isDraggingSplitter) {
			_isDraggingSplitter = false;
			ref<Settings> st = GetSettings();
			if(st) {
				st->SetValue(L"names.width", Stringify(_nameWidth));
			}

			Layout();
			ReleaseCapture();
		}
	}
	else if(ev==MouseEventLDown) {
		if(x>(_nameWidth-5) && x<(_nameWidth+5)) {
			_isDraggingSplitter = true;
			SetCapture(GetWindow());
		}
		else if(x>(_nameWidth-21) && x<(_nameWidth-5)) {
			// Expander/collapse icon
			// Find the property at that location and collapse/expand it
			// TODO: move to separate function
			y -= GetPathHeight();
			int cH = -GetVerticalPos();
			std::vector<ref<Property> >::iterator it = _properties.begin();

			while(it!=_properties.end()) {
				ref<Property> pr = *it;
				if(pr) {
					Pixels h = pr->GetHeight();
					if(y>cH && y<cH+h) {
						// found
						if(pr->IsExpandable()) {
							pr->SetExpanded(!pr->IsExpanded());
						}
						OnSize(GetClientArea()); // So our scrollbar is updated
						Layout();
						Repaint();
						return;
					}
					cH += h + 6; // TODO: make the 6 a constant (see the Paint code for more info)
				}
				++it;
			}
		}
	}
	else if(ev==MouseEventMove) {
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
}

LRESULT PropertyGridWnd::Message(UINT msg, WPARAM wParam, LPARAM lParam) {
	if(msg==WM_CLOSE) {
		ShowWindow(GetWindow(),SW_HIDE);
		return 1;
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
				++it;
			}
		}
		else if(HIWORD(wParam)==EN_SETFOCUS) {
			HWND he = (HWND)lParam;

			if(_showHints) {
				// Show hint for this property, if there is a hint
				std::vector< ref<Property> >::iterator it = _properties.begin();
				while(it!=_properties.end()) {
					ref<Property> prw = (*it);
					assert(prw!=0);

					if(prw->GetWindow()==he) {
						const std::wstring& hint = prw->GetHint();
			
						if(hint.length()>0) {
							std::wstring name = prw->GetName();
							EDITBALLOONTIP ebt;
							ebt.cbStruct = sizeof(ebt);
							ebt.pszTitle = name.c_str();
							ebt.pszText = hint.c_str();
							ebt.ttiIcon = TTI_INFO;
							Edit_ShowBalloonTip(he, &ebt);
						}
						break;
					}
					++it;
				}
			}

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

	return ChildWnd::Message(msg,wParam, lParam);
}

Pixels PropertyGridWnd::GetPathHeight() const {
	return _path ? KPathHeight : 0;
}

void PropertyGridWnd::Layout() {
	Area rect = GetClientArea();

	if(_path) {
		_path->Move(0,0,rect.GetWidth(), KPathHeight);
	}

	float df = ThemeManager::GetTheme()->GetDPIScaleFactor();

	Pixels cH = GetPathHeight() - GetVerticalPos();
	std::vector<ref<Property> >::iterator it = _properties.begin();

	while(it!=_properties.end()) {
		ref<Property> pr = *it;
		if(pr) {
			HWND vw = pr->GetWindow();
			Pixels h = pr->GetHeight();

			SendMessage(vw, WM_SETFONT, (WPARAM)(HFONT)_editFont, FALSE);
			LONG style = GetWindowLong(vw, GWL_STYLE);
			style |= WS_CLIPSIBLINGS;
			
			// Some nice DPI conversions...
			SetWindowLong(vw, GWL_STYLE, style);
			SetWindowPos(vw, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			SetWindowPos(vw, 0,  int(ceil(_nameWidth*df)), int(ceil((cH+3)*df)),  int(ceil((rect.GetWidth()-_nameWidth-3)*df)), int(ceil(h*df)), SWP_NOZORDER);
			ShowWindow(vw, SW_SHOW);
			cH += h + 6; // TODO: make the 6 a constant
		}
		++it;
	}
	Repaint();
}

void PropertyGridWnd::OnSize(const Area& ns) {
	ReplyMessage(0);

	// Get total property height
	int totalHeight = 0;
	std::vector< ref<Property> >::iterator it = _properties.begin();
	while(it!=_properties.end()) {
		ref<Property> pr = *it;
		if(pr) {
			totalHeight += pr->GetHeight() + 6;
		}
		++it;
	}

	SetVerticalScrollInfo(Range<int>(0, totalHeight+GetPathHeight()), ns.GetHeight());
	Layout();
	Repaint();
}

void PropertyGridWnd::Update() {
	std::vector< ref<Property> >::iterator it = _properties.begin();
	while(it!=_properties.end()) {
		ref<Property> pr = (*it);
		pr->Update();
		++it;
	}

	if(_path) {
		_path->Update();
	}
	Repaint();
}

void PropertyGridWnd::Clear() {
	_properties.clear();
	Layout();
}

void PropertyGridWnd::Inspect(Inspectable* isp, ref<Path> p) {
	_properties.clear();

	if(isp!=0) {
		ref<PropertySet> propset = isp->GetProperties();
		if(propset) {
			HWND myself = GetWindow();
			HWND first = 0;

			// Iterate over properties, create their windows and store the properties on our own list
			std::vector< ref<Property> >::iterator it = propset->_properties.begin();
			std::vector< ref<Property> >::iterator end = propset->_properties.end();

			while(it!=end) {
				ref<Property> pr = *it;

				if(pr) {
					HWND f = pr->Create(myself);

					if(first==0) {
						first = f;
					}

					_properties.push_back(pr);
				}
				++it;
			}

			// Update geometry
			if(_path) {
				_path->SetPath(p);
			}
			SetFocus(first);
			ClearThemeCache();
			SetVerticalPos(0);
			OnSize(GetClientArea());
			return;
		}
	}
	
	// Otherwise, make it empty
	ClearThemeCache();
	if(_path) {
		_path->SetPath(0);
	}
	OnSize(GetClientArea());
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