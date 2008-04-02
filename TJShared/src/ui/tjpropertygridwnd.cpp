#include "../../include/tjshared.h"
#include <windowsx.h>
#include <commctrl.h>
using namespace tj::shared;
using namespace Gdiplus;

const Pixels PropertyGridWnd::KPathHeight = 24;
const Pixels PropertyGridWnd::KMinimumNameColumnWidth = 10;
const Pixels PropertyGridWnd::KPropertyMargin = 3;

/// TODO: Alles netjes Pixels maken ipv int (ook scrollbar), een GetPropertyAt(y=?) maken en de expand/collapse
/// code fixen (en misschien sommige properties die nog niet met Pixels werken even fixen).

/* PropertyGridWnd implementation */
PropertyGridWnd::PropertyGridWnd(bool withPath): ChildWnd(TL(properties)),
_expandIcon(Icons::GetIconPath(Icons::IconExpand)), _collapseIcon(Icons::GetIconPath(Icons::IconCollapse)) {
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

void PropertyGridWnd::FocusFirstProperty() {
	if(_properties.size()>0) {
		ref<Property> p = (*_properties.begin());
		SetFocus(p->GetWindow());
	}
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
	bool previousCollapsed = false;

	while(it!=_properties.end()) {
		ref<Property> p = *it;
		if(!p) {
			++it; 
			continue;
		}

		bool isSeparator = p.IsCastableTo<PropertySeparator>();
		if(isSeparator) {
			ref<PropertySeparator> sep = p;
			previousCollapsed = sep->IsCollapsed();
		}

		p->Update();
		if(isSeparator || !previousCollapsed) {
			if(GetFocus()==p->GetWindow()) {
				LinearGradientBrush gbr(Gdiplus::Point(0, cH), Gdiplus::Point(0, cH+p->GetHeight()+10), theme->GetTimeSelectionColorStart(), theme->GetTimeSelectionColorEnd());
				g.FillRectangle(&gbr, Rect(1, cH+1, r.GetWidth()-2, p->GetHeight()+(2*KPropertyMargin)-2));
			}

			if(isSeparator) {
				LinearGradientBrush gbr(Gdiplus::Point(0, cH), Gdiplus::Point(0, cH+p->GetHeight()+10), theme->GetActiveStartColor(), theme->GetActiveEndColor());
				g.FillRectangle(&gbr, Rect(1, cH+1, r.GetWidth()-2, p->GetHeight()+(2*KPropertyMargin)-2));

				SolidBrush dbr(theme->GetDisabledOverlayColor());
				g.FillRectangle(&dbr, Rect(1, cH+1, r.GetWidth()-2, p->GetHeight()+(2*KPropertyMargin)-2));
			}

			std::wstring ws = p->GetName();
			SolidBrush tb(theme->GetTextColor());
			
			g.DrawString(ws.c_str(), (int)ws.length(),isSeparator?theme->GetGUIFontBold():theme->GetGUIFont(), PointF(stringLeft+(isSeparator?(KPropertyMargin+16):0), float(cH+5)), &tb);
			
			// Draw collapse/expand icon for separator
			if(isSeparator) {
				Area expander(KPropertyMargin, cH+KPropertyMargin, 16, 16);
				ref<PropertySeparator> sep = p;
				g.DrawImage(sep->IsCollapsed()?_expandIcon:_collapseIcon, expander);
			}
			// Draw collapse/expand icon if this property is collapsable
			else if(p->IsExpandable()) {
				Area expander(_nameWidth-21, cH+KPropertyMargin, 16, 16);
				g.DrawImage(p->IsExpanded()?_collapseIcon:_expandIcon, expander);
			}
			
			cH += p->GetHeight() + 2*KPropertyMargin;
		}

		++it;
		hI++;
	}
}

void PropertyGridWnd::OnScroll(ScrollDirection dir) {
	Layout();
	UpdateWindow(GetWindow());
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
		Focus();

		if(x>(_nameWidth-5) && x<(_nameWidth+5)) {
			_isDraggingSplitter = true;
			SetCapture(GetWindow());
		}
		else {
			// Expander/collapse icon
			// Find the property at that location and collapse/expand it
			// TODO: move to separate function
			y -= GetPathHeight();
			int cH = -GetVerticalPos();
			std::vector<ref<Property> >::iterator it = _properties.begin();
			bool previousCollapsed = false;

			while(it!=_properties.end()) {
				ref<Property> pr = *it;
				Pixels h = pr->GetHeight();

				if(pr) {
					if(pr.IsCastableTo<PropertySeparator>()) {
						previousCollapsed = ref<PropertySeparator>(pr)->IsCollapsed();
						if(y>cH && y<cH+h) {
							// found, clicked a separator
							ref<PropertySeparator> sep = pr;
							sep->SetCollapsed(!sep->IsCollapsed());

							OnSize(GetClientArea()); // So our scrollbar is updated
							Layout();
							Repaint();
							Layout();
							return;
						}
						cH += h + 2*KPropertyMargin;
					}
					else if(!previousCollapsed) {
						if(y>cH && y<cH+h) {
							if(x>(_nameWidth-21) && x<(_nameWidth-5)) { // expand/collapse icons have this x
								// found
								if(pr->IsExpandable()) {
									pr->SetExpanded(!pr->IsExpanded());
								}
								OnSize(GetClientArea()); // So our scrollbar is updated
								Layout();
								Repaint();
								return;
							}
						}
						cH += h + 2*KPropertyMargin;
					}
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
		if(HIWORD(wParam)==EN_UPDATE||HIWORD(wParam)==BN_CLICKED||HIWORD(wParam)==CBN_SELCHANGE||HIWORD(wParam)==CBN_EDITCHANGE) {
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
		
		if(HIWORD(wParam)==EN_CHANGE || HIWORD(wParam)==BN_CLICKED||HIWORD(wParam)==CBN_SELCHANGE) {
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

	bool previousCollapsed = false;
	while(it!=_properties.end()) {
		ref<Property> pr = *it;
		if(pr) {
			Pixels h = pr->GetHeight();

			if(pr.IsCastableTo<PropertySeparator>()) {
				previousCollapsed = ref<PropertySeparator>(pr)->IsCollapsed();
				cH += h + 2*KPropertyMargin;
			}
			else {
				HWND vw = pr->GetWindow();

				if(!previousCollapsed) {
					SendMessage(vw, WM_SETFONT, (WPARAM)(HFONT)_editFont, FALSE);
					LONG style = GetWindowLong(vw, GWL_STYLE);
					style |= WS_CLIPSIBLINGS;
					
					// Some nice DPI conversions...
					SetWindowLong(vw, GWL_STYLE, style);
					SetWindowPos(vw, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
					SetWindowPos(vw, 0,  int(ceil(_nameWidth*df)), int(ceil((cH+KPropertyMargin)*df)),  int(ceil((rect.GetWidth()-_nameWidth-KPropertyMargin)*df)), int(ceil(h*df)), SWP_NOZORDER);
					ShowWindow(vw, SW_SHOW);
					cH += h + 2*KPropertyMargin;
				}
				else {
					ShowWindow(vw, SW_HIDE);
				}
			}
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
	bool previousCollapsed = false;
	while(it!=_properties.end()) {
		ref<Property> pr = *it;
		if(pr) {
			if(pr.IsCastableTo<PropertySeparator>()) {
				previousCollapsed = ref<PropertySeparator>(pr)->IsCollapsed();
				totalHeight += pr->GetHeight() + 2*KPropertyMargin;
			}

			if(!previousCollapsed) {
				totalHeight += pr->GetHeight() + 2*KPropertyMargin;
			}
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
	_subject = 0;
	if(_path) {
		_path->SetPath(0);
	}

	Layout();
}

// Clearly, this is NOT threadsafe (but it doesn't have to be anyway)
void PropertyGridWnd::Inspect(ref<Inspectable> isp, ref<Path> p) {
	_properties.clear();
	_subject = isp;

	if(isp) {
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
		}
	}
	
	if(_path) {
		_path->SetPath(p);
	}

	ClearThemeCache();
	SetVerticalPos(0);
	OnSize(GetClientArea());
}