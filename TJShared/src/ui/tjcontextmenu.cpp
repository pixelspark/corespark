#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

const Pixels ContextPopupWnd::KItemHeight = 19;
const Pixels ContextPopupWnd::KMinContextMenuWidth = 150;
const unsigned int ContextPopupWnd::KMaxItems = 15;

Menu::~Menu() {
}

ContextMenu::ContextMenu() {
}

ContextMenu::~ContextMenu() {
}

/* If x=-1 and y=-1, we use the current cursor pos */
int ContextMenu::DoContextMenu(ref<Wnd> wnd, Pixels x, Pixels y) {
	// If there are no items, always fail
	if(_items.empty()) {
		return -1;
	}
	
	if(wnd) {
		// If this context menu is called from a popup window, disable it,
		// so it won't disappear when it is deactivated
		HWND root = ::GetAncestor(wnd->GetWindow(), GA_ROOT);
		DWORD style = GetWindowLong(root, GWL_STYLE);
		bool fromPopup = ((style & WS_POPUP) !=0);
		if(fromPopup) {
			///Log::Write(L"TJShared/ContextMenu", L"Owner of this context menu is a popup");
			EnableWindow(root, FALSE);
		}

		if(x<0 || y<0) {
			ref<Theme> theme = ThemeManager::GetTheme();
			float df = theme->GetDPIScaleFactor();
			POINT px;
			if(GetCursorPos(&px)) {
				ScreenToClient(wnd->GetWindow(), &px);
				if(x<0) {
					x = Pixels(float(px.x) / df);
				}
				if(y<0) {
					y = Pixels(float(px.y) / df);
				}
			}
		}

		// Create popup
		ref<ContextPopupWnd> cpw = GC::Hold(new ContextPopupWnd(this, wnd->GetWindow()));

		// Calculate size of text
		int result =  cpw->DoModal(wnd,x,y);

		if(fromPopup) {
			EnableWindow(root, TRUE);
		}
		SetForegroundWindow(wnd->GetWindow());

		return result;
	}

	Throw(L"Context menu must have an owner window", ExceptionTypeSevere);
}

int ContextMenu::DoContextMenu(ref<Wnd> wnd) {
	if(wnd) {	
		Area  rc = wnd->GetClientArea();
		return DoContextMenu(wnd, -1, -1);
	}

	Throw(L"Context menu must have an owner window", ExceptionTypeSevere);
}

/** if command == -1, then the item will be grayed out/disabled **/
void ContextMenu::AddItem(const std::wstring& name, int command, bool hilite, bool radiocheck) {
	AddItem(name,command,hilite, radiocheck?MenuItem::Checked: MenuItem::NotChecked);
}

void ContextMenu::AddItem(const std::wstring& name, int command, bool hilite, MenuItem::CheckType checked) {
	ref<MenuItem> ci = GC::Hold(new MenuItem(name,command,hilite,checked));
	AddItem(ci);
}

void ContextMenu::AddItem(ref<MenuItem> ci) {
	if(!ci) Throw(L"Cannot add a null context item", ExceptionTypeError);
	_items.push_back(ci);

	if(ci->GetTitle().length() > _longestString.length()) {
		_longestString = ci->GetTitle();
	}
}

void ContextMenu::AddSeparator(const std::wstring& title) {
	ref<MenuItem> item = GC::Hold(new MenuItem());
	item->SetTitle(title);
	item->SetSeparator(true);
	_items.push_back(item);
}

/** ContextPopupWnd **/
ContextPopupWnd::ContextPopupWnd(ContextMenu* cm, HWND parent): PopupWnd(parent,false), _cm(cm), _result(-1), _mouseOver(-1), _mouseDown(-1), _checkedIcon(Icons::GetIconPath(Icons::IconChecked)), _radioCheckedIcon(Icons::GetIconPath(Icons::IconRadioChecked)) {
	SetWantMouseLeave(true);
	SetVerticallyScrollable(true);
}

ContextPopupWnd::~ContextPopupWnd() {
}

int ContextPopupWnd::DoModal(ref<Wnd> parent, Pixels x, Pixels y) {
	ref<Theme> theme = ThemeManager::GetTheme();
	Area measured = theme->MeasureText(_cm->_longestString, theme->GetGUIFontBold());
	Pixels width = max(KMinContextMenuWidth, measured.GetWidth()+10)+KItemHeight;

	// Showtime
	unsigned int itemsShown = min((unsigned int)_cm->_items.size(), ContextPopupWnd::KMaxItems);
	SetSize(width, Pixels(int(itemsShown)*KItemHeight));
	
	// Set scroll info
	SetVerticalScrollInfo(Range<int>(0,int(_cm->_items.size())*KItemHeight), itemsShown*KItemHeight + 1);
	SetVerticalPos(0);

	PopupAt(x,y,parent);

	// Start modality
	_result = -1;
	ModalLoop::Result res = _loop.Enter(GetWindow(),false);
	Show(false);

	if(res==ModalLoop::ResultSucceeded) {
		return _result;
	}

	return -1;
}

void ContextPopupWnd::EndModal(int r) {
	_result = r;
	_loop.End(ModalLoop::ResultSucceeded);
}

int ContextPopupWnd::GetItemAt(Pixels y) {
	return ((y+GetVerticalPos())/KItemHeight);
}

void ContextPopupWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventMove || ev==MouseEventLDown) {
		int idx = GetItemAt(y);
		if(idx>=0 && idx<int(_cm->_items.size())) {
			_mouseOver = idx;
			if(ev==MouseEventLDown) {
				_mouseDown = idx;
			}
			else {
				_mouseDown = -1;
			}
		}
		else {
			_mouseOver = -1;
			_mouseDown = -1;
		}
		Repaint();
	}
	else if(ev==MouseEventLUp) {
		int idx = GetItemAt(y);
		if(idx>=0 && idx<int(_cm->_items.size())) {
			ref<MenuItem> ci = _cm->_items.at(idx);
			if(!ci->IsDisabled() || ci->IsSeparator()) {
				int command = _cm->_items.at(idx)->_command;
				EndModal(command);
			}
		}
	}
	else if(ev==MouseEventLeave) {
		_mouseOver = -1;
		Repaint();
	}
	PopupWnd::OnMouse(ev,x,y);
}

void ContextPopupWnd::OnActivate(bool a) {
	if(!a) {
		_loop.End(ModalLoop::ResultCancelled);
	}
	else {
		PopupWnd::OnActivate(a);
	}
}

void ContextPopupWnd::OnKey(Key k, wchar_t ch, bool down, bool accelerator) {
	if(!accelerator) {
		switch(k) {
			case KeyDown:
				if(down) {
					++_mouseOver;
					if(_mouseOver > int(_cm->_items.size())-1) {
						_mouseOver = 0;
					}
					_mouseDown = -1;
				}
				break;

			case KeyUp:
				if(down) {
					--_mouseOver;
					if(_mouseOver < 0) {
						_mouseOver = int(_cm->_items.size())-1;
					}
					_mouseDown = -1;
				}
				break;
			case KeyCharacter:
				if(ch==VK_SPACE) {
					if(down) {
						_mouseDown = _mouseOver;
					}
					else {
						if(_mouseDown >= 0 && _mouseDown < int(_cm->_items.size())) {
							EndModal(_cm->_items.at(_mouseDown)->_command);
						}
					}
				}
				break;
		}
	}
	Repaint();
}

void ContextPopupWnd::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();
	SolidBrush back(theme->GetBackgroundColor());
	g.FillRectangle(&back, rc);

	Pixels y = -GetVerticalPos();

	SolidBrush text(theme->GetTextColor());
	SolidBrush header(theme->GetActiveStartColor());
	SolidBrush link(theme->GetLinkColor());
	SolidBrush disabled(theme->GetDisabledOverlayColor());
	SolidBrush colorSeparator(theme->GetActiveEndColor());
	Pen separator(&colorSeparator, 1.0f);
	LinearGradientBrush selected(PointF(0.0f, float(y)), PointF(0.0f, float(y+KItemHeight)), theme->GetTimeSelectionColorStart(), theme->GetTimeSelectionColorEnd());
	LinearGradientBrush down(PointF(0.0f, float(y)), PointF(0.0f, float(y+KItemHeight)), theme->GetTimeSelectionColorEnd(), theme->GetTimeSelectionColorStart());
	
	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);
	sf.SetLineAlignment(StringAlignmentCenter);
	sf.SetTrimming(StringTrimmingEllipsisPath);
	REAL tabs[] = {50, 50, 50};
	sf.SetTabStops(0, 3, tabs);

	int n = 0;
	std::vector< ref<MenuItem> >::iterator it = _cm->_items.begin();
	while(it!=_cm->_items.end()) {
		ref<MenuItem> item = *it;
		Area current = rc;
		current.SetY(y);
		current.SetHeight(KItemHeight);

		Area currentText = current;
		

		if(item->IsSeparator()) {
			const std::wstring& title = item->GetTitle();
			if(title.length()>0) {
				currentText.Narrow(KItemHeight/2,2,2,2);
				g.DrawString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), currentText, &sf, &header);
				RectF lr;
				g.MeasureString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), currentText, &sf, &lr);
				current.Narrow((Pixels)lr.Width, 0,0,0);
			}

			current.Narrow(KItemHeight,0,KItemHeight,0);
			Pixels y = current.GetTop() + (current.GetBottom()-current.GetTop())/2;
			g.DrawLine(&separator, current.GetLeft(), y, current.GetRight(), y);
		}
		else {
			currentText.Narrow(KItemHeight,2,2,2);

			if(n==_mouseDown) {
				Area selection = current;
				selection.Narrow(1,0,1,0);
				g.FillRectangle(&down, selection);
			}
			else if(n==_mouseOver) {
				Area selection = current;
				selection.Narrow(1,0,1,0);
				g.FillRectangle(&selected, selection);
			}

			if(item->IsLink()) {
				g.DrawString(item->GetTitle().c_str(), (int)item->GetTitle().length(), theme->GetLinkFont(), currentText, &sf, &link);
			}
			else {
				g.DrawString(item->GetTitle().c_str(), (int)item->GetTitle().length(), item->_hilite?theme->GetGUIFontBold():theme->GetGUIFont(), currentText, &sf, &text);
			}

			if(item->HasIcon() || item->_checked!=MenuItem::NotChecked) {
				Area iconArea = current;
				iconArea.Narrow(3,1,1,1);
				iconArea.SetWidth(16);
				iconArea.SetHeight(16);
				if(item->HasIcon()) {
					g.DrawImage(item->GetIcon()->GetBitmap(), iconArea);
				}
				else {
					g.DrawImage((item->_checked == MenuItem::Checked)?_checkedIcon:_radioCheckedIcon, iconArea);
				}
			}

			if(item->IsDisabled()) {
				g.FillRectangle(&disabled, current);
			}
		}

		y += KItemHeight;
		++it;
		++n;
	}

	// Draw border
	rc.Narrow(0,0,1,1);
	SolidBrush border(theme->GetActiveStartColor());
	Pen pn(&border, 1.0f);
	g.DrawRectangle(&pn, rc);
}

/** MenuItem */
MenuItem::MenuItem(): _separator(true), _hilite(false), _link(false), _checked(MenuItem::NotChecked), _command(0), _icon(0) {
}

MenuItem::MenuItem(const std::wstring& title, int command, bool highlight, MenuItem::CheckType checked, const std::wstring& icon): _title(title), _command(command), _hilite(highlight), _checked(checked), _separator(false), _icon(0), _link(false) {
	if(icon.length()>0) {
		SetIcon(icon);
	}
}

MenuItem::MenuItem(const std::wstring& title, int command, bool highlight, MenuItem::CheckType ct, ref<Icon> icon): _title(title), _command(command), _hilite(highlight), _checked(ct), _separator(false), _icon(icon), _link(false) {
}

MenuItem::~MenuItem() {
}

bool MenuItem::IsLink() const {
	return _link;
}

void MenuItem::SetLink(bool l) {
	_link = l;
}

bool MenuItem::IsDisabled() const {
	return _command == -1;
}

bool MenuItem::IsSeparator() const {
	return _separator;
}

const std::wstring& MenuItem::GetTitle() const {
	return _title;
}

ref<Icon> MenuItem::GetIcon() {
	return _icon;
}

bool MenuItem::HasIcon() const {
	return _icon!=0;
}

void MenuItem::SetIcon(const std::wstring& icon) {
	_icon = GC::Hold(new Icon(icon));
}

void MenuItem::SetTitle(const std::wstring& title) {
	_title = title;
}

void MenuItem::SetSeparator(bool s) {
	_separator = s;
}