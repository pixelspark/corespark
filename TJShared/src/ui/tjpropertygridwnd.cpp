#include "../../include/ui/tjui.h" 
#include "../../include/properties/tjproperties.h"
#include <windowsx.h>
#include <commctrl.h>
using namespace tj::shared;
using namespace tj::shared::graphics;

const Pixels PropertyGridWnd::KPathHeight = 24;
const Pixels PropertyGridWnd::KMinimumNameColumnWidth = 10;
const Pixels PropertyGridWnd::KPropertyMargin = 3;

/// TODO: Alles netjes Pixels maken ipv int (ook scrollbar), een GetPropertyAt(y=?) maken en de expand/collapse
/// code fixen (en misschien sommige properties die nog niet met Pixels werken even fixen).

/* PropertyGridWnd implementation */
PropertyGridWnd::PropertyGridWnd(bool withPath): ChildWnd(TL(properties)),
_expandIcon(Icons::GetIconPath(Icons::IconExpand)), _collapseIcon(Icons::GetIconPath(Icons::IconCollapse)),
_nameWidth(100), _isDraggingSplitter(false), _showHints(true) {
	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	SetStyleEx(WS_EX_CONTROLPARENT);
	SetVerticallyScrollable(true);
	_tw = GC::Hold(new TooltipWnd(GetWindow()));

	if(withPath) {
		_path = GC::Hold(new PathWnd(this));
		Add(_path);
	}
	Layout();
}

PropertyGridWnd::~PropertyGridWnd() {
}

void PropertyGridWnd::FocusFirstProperty() {
	if(_properties.size()>0) {
		ref<Property> p = (*_properties.begin());
		ref<Wnd> w = p->GetWindow();
		if(w) {
			w->Focus();
		}
	}
}

void PropertyGridWnd::SetNameWidth(Pixels w) {
	_nameWidth = w;
	Repaint();
}

void PropertyGridWnd::SetShowHints(bool t) {
	_showHints = t;
	ref<Settings> st = GetSettings();
	if(st) {
		st->SetValue(L"hints.show", _showHints?L"yes":L"no");
	}
	Repaint();
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

void PropertyGridWnd::Paint(Graphics& g, strong<Theme> theme) {
	Area r = GetClientArea();

	SolidBrush br(theme->GetColor(Theme::ColorPropertyBackground));
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
			if(p->GetWindow() && p->GetWindow()->HasFocus()) {
				LinearGradientBrush gbr(graphics::Point(0, cH), graphics::Point(0, cH+p->GetHeight()+10), theme->GetColor(Theme::ColorTimeSelectionStart), theme->GetColor(Theme::ColorTimeSelectionEnd));
				g.FillRectangle(&gbr, Rect(1, cH+1, r.GetWidth()-2, p->GetHeight()+(2*KPropertyMargin)-2));
			}

			if(isSeparator) {
				LinearGradientBrush gbr(graphics::Point(0, cH), graphics::Point(0, cH+p->GetHeight()+10), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
				g.FillRectangle(&gbr, Rect(1, cH+1, r.GetWidth()-2, p->GetHeight()+(2*KPropertyMargin)-2));

				SolidBrush dbr(theme->GetColor(Theme::ColorDisabledOverlay));
				g.FillRectangle(&dbr, Rect(1, cH+1, r.GetWidth()-2, p->GetHeight()+(2*KPropertyMargin)-2));
			}

			std::wstring ws = p->GetName();
			SolidBrush tb(theme->GetColor(Theme::ColorText));
			
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
		_tw->SetTrackEnabled(false);

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
							// found; focus this property
							ref<Wnd> pw = pr->GetWindow();
							if(pw) {
								pw->Focus();
							}

							// Expand/collapse icon
							if(x>(_nameWidth-21) && x<(_nameWidth-5)) { // expand/collapse icons have this x
								if(pr->IsExpandable()) {
									pr->SetExpanded(!pr->IsExpanded());
								}
								OnSize(GetClientArea()); // So our scrollbar is updated
								Layout();
							}
							else {
								// Show the hint, if the property has one and we are allowed to show them
								if(_showHints) {
									const std::wstring& hint = pr->GetHint();
									if(hint.length()>0) {
										_tw->SetTooltip(hint);
										_tw->Move(_nameWidth,cH+h+GetPathHeight()+2*KPropertyMargin);
										_tw->SetTrackEnabled(true);

									}
								}
							}

							Repaint();
							return;
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

Pixels PropertyGridWnd::GetPathHeight() const {
	return _path ? KPathHeight : 0;
}

void PropertyGridWnd::Layout() {
	Area rect = GetClientArea();

	if(_path) {
		_path->Move(0,0,rect.GetWidth(), KPathHeight);
		// Make it topmost
		SetWindowPos(_path->GetWindow(), HWND_TOP, 0,0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
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
				ref<Wnd> w = pr->GetWindow();

				if(w) {
					if(!previousCollapsed) {					
						// Some nice DPI conversions...
						//SetWindowPos(vw, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
						w->Move(Pixels(_nameWidth), Pixels(cH+KPropertyMargin), Pixels(rect.GetWidth()-_nameWidth-KPropertyMargin), Pixels(h));
						cH += h + 2*KPropertyMargin;
						w->Show(true);
					}
					else {
						w->Show(false);
					}
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
	_subject = null;
	if(_path) {
		_path->SetPath(null);
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
			ref<Wnd> first;

			// Iterate over properties, create their windows and store the properties on our own list
			std::vector< ref<Property> >::iterator it = propset->_properties.begin();
			std::vector< ref<Property> >::iterator end = propset->_properties.end();

			while(it!=end) {
				ref<Property> pr = *it;

				if(pr) {
					ref<Wnd> w = pr->GetWindow();
					Add(w,false);

					if(!first) {
						first = w;
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

	SetVerticalPos(0);
	OnSize(GetClientArea());
}