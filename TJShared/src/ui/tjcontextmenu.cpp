#include "../../include/ui/tjui.h" 
using namespace tj::shared;
using namespace tj::shared::graphics;

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
	_openAnimation.SetLength(Time(300));
	_closeAnimation.SetLength(Time(200));
	_openAnimation.SetEase(Animation::EaseQuadratic);
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

	StartTimer(Time(50), 1);
	_openAnimation.Start();
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
	if(Animation::IsAnimationsEnabled() && _result == -1) {
		_result = r;
		_closeAnimation.Start();
		StartTimer(Time(50), 2);
	}
	else {
		_result = r;
		_loop.End(ModalLoop::ResultSucceeded);
	}
}

int ContextPopupWnd::GetItemAt(Pixels y) {
	return ((y+GetVerticalPos())/KItemHeight);
}

void ContextPopupWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventMove || ev==MouseEventLDown) {
		if(!_closeAnimation.IsAnimating()) {
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
		if(!_closeAnimation.IsAnimating()) {
			_mouseOver = -1;
			Repaint();
		}
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

void ContextPopupWnd::Paint(graphics::Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();
	SolidBrush back(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&back, rc);

	Pixels y = -GetVerticalPos();

	// Get us some colors
	SolidBrush text(theme->GetColor(Theme::ColorText));
	SolidBrush header(theme->GetColor(Theme::ColorActiveStart));
	SolidBrush link(theme->GetColor(Theme::ColorLink));
	SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
	SolidBrush closing(Theme::ChangeAlpha(theme->GetColor(Theme::ColorDisabledOverlay),_closeAnimation.GetFraction()));
	SolidBrush colorSeparator(theme->GetColor(Theme::ColorActiveEnd));
	Pen separator(&colorSeparator, 1.0f);
	LinearGradientBrush selected(PointF(0.0f, float(y)), PointF(0.0f, float(y+KItemHeight)), theme->GetColor(Theme::ColorTimeSelectionStart), theme->GetColor(Theme::ColorTimeSelectionEnd));
	LinearGradientBrush down(PointF(0.0f, float(y)), PointF(0.0f, float(y+KItemHeight)), theme->GetColor(Theme::ColorTimeSelectionEnd), theme->GetColor(Theme::ColorTimeSelectionStart));
	
	// Set up string format
	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);
	sf.SetLineAlignment(StringAlignmentCenter);
	sf.SetTrimming(StringTrimmingEllipsisPath);
	REAL tabs[] = {50, 50, 50};
	sf.SetTabStops(0, 3, tabs);

	int n = 0;
	Pixels indentSize = Pixels(KItemHeight*0.619f);

	std::vector< ref<MenuItem> >::iterator it = _cm->_items.begin();
	while(it!=_cm->_items.end()) {
		ref<MenuItem> item = *it;
		Area current = rc;
		current.SetY(y);
		current.SetHeight(KItemHeight);

		Pixels indentRight = item->GetIndent()*indentSize;
		current.Narrow(indentRight,0,0,0);
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

			// Draw item text
			if(item->IsLink()) {
				g.DrawString(item->GetTitle().c_str(), (int)item->GetTitle().length(), theme->GetLinkFont(), currentText, &sf, &link);
			}
			else {
				g.DrawString(item->GetTitle().c_str(), (int)item->GetTitle().length(), item->_hilite?theme->GetGUIFontBold():theme->GetGUIFont(), currentText, &sf, &text);
			}

			// Draw item icon or check mark
			if(item->HasIcon() || item->_checked!=MenuItem::NotChecked) {
				Area iconArea = current;
				iconArea.Narrow(3,1,1,1);
				iconArea.SetWidth(16);
				iconArea.SetHeight(16);
				if(item->HasIcon()) {
					item->GetIcon()->Paint(g, iconArea, !item->IsDisabled(), _openAnimation.GetFraction());
				}
				else {
					Icon& icon = (item->_checked == MenuItem::Checked)?_checkedIcon:_radioCheckedIcon;
					icon.Paint(g, iconArea, !item->IsDisabled(), _openAnimation.GetFraction());
				}
			}

			// Draw hotkey label
			if(item->HasHotkey()) {
				StringFormat hsf;
				hsf.SetLineAlignment(StringAlignmentCenter);
				hsf.SetAlignment(StringAlignmentFar);

				const std::wstring& hk = item->GetHotkey();
				RectF hbb;
				g.MeasureString(hk.c_str(), (int)hk.length(), theme->GetGUIFontSmall(), PointF(float(current.GetTop()), float(current.GetRight())), &hsf, &hbb);

				Pixels w = Pixels(hbb.Width) + 6;
				Area hotkeyLabel(current.GetRight()-w, current.GetTop(), w, current.GetHeight());
				hotkeyLabel.Narrow(2,2,2,3);
				hotkeyLabel.Translate(-2, 0);

				SolidBrush htbr(theme->GetColor(/*** n==_mouseOver ? Theme::ColorBackground : ***/ Theme::ColorActiveEnd));
				g.DrawString(hk.c_str(), (int)hk.length(), theme->GetGUIFontSmall(), hotkeyLabel, &hsf, &htbr);
			}

			if(item->IsDisabled()) {
				g.FillRectangle(&disabled, current);
			}

			if(_closeAnimation.IsAnimating() && _result != item->_command) {
				g.FillRectangle(&closing, current);
			}
		}

		y += KItemHeight;
		++it;
		++n;
	}

	// Draw border
	rc.Narrow(0,0,1,1);
	SolidBrush border(theme->GetColor(Theme::ColorActiveStart));
	Pen pn(&border, 1.0f);
	g.DrawRectangle(&pn, rc);
}

void ContextPopupWnd::OnTimer(unsigned int id) {
	if(!_closeAnimation.IsAnimating() && _result!=-1) {
		StopTimer(2);
		EndModal(_result);
	}
	
	if(!_openAnimation.IsAnimating()) {
		StopTimer(1);
	}
	Repaint();
}

/** MenuItem */
MenuItem::MenuItem(): _separator(true), _hilite(false), _link(false), _checked(MenuItem::NotChecked), _command(0), _icon(0), _indent(0) {
}

MenuItem::MenuItem(const std::wstring& title, int command, bool highlight, MenuItem::CheckType checked, const std::wstring& icon, const std::wstring& hotkey): _title(title), _command(command), _hilite(highlight), _checked(checked), _separator(false), _icon(0), _link(false), _indent(0), _hotkey(hotkey) {
	if(icon.length()>0) {
		SetIcon(icon);
	}
}

MenuItem::MenuItem(const std::wstring& title, int command, bool highlight, MenuItem::CheckType ct, ref<Icon> icon): _title(title), _command(command), _hilite(highlight), _checked(ct), _separator(false), _icon(icon), _link(false), _indent(0) {
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
	return _icon!=null;
}

bool MenuItem::HasHotkey() const {
	return _hotkey.length()>0;
}

void MenuItem::SetIcon(const std::wstring& icon) {
	_icon = GC::Hold(new Icon(icon));
}

void MenuItem::SetIcon(ref<Icon> icon) {
	_icon = icon;
}

void MenuItem::SetTitle(const std::wstring& title) {
	_title = title;
}

void MenuItem::SetSeparator(bool s) {
	_separator = s;
}

unsigned char MenuItem::GetIndent() const {
	return _indent;
}

void MenuItem::SetIndent(unsigned char c) {
	_indent = c;
}

void MenuItem::SetHotkey(const std::wstring& hk) {
	_hotkey = hk;
}

const std::wstring& MenuItem::GetHotkey() const {
	return _hotkey;
}