#include "../../include/ui/tjui.h" 
using namespace tj::shared;
using namespace tj::shared::graphics;

const unsigned int ContextPopupWnd::KMaxItems = 15;

Menu::~Menu() {
}

ContextMenu::ContextMenu(): _menu(GC::Hold(new BasicMenu())) {
}

ContextMenu::~ContextMenu() {
}

/* If x=-1 and y=-1, we use the current cursor pos */
int ContextMenu::DoContextMenu(ref<Wnd> wnd, Pixels x, Pixels y) {
	ref<MenuItem> mi = DoContextMenuByItem(wnd, x, y);
	if(mi) {
		return mi->GetCommandCode();
	}
	return -1;
}

ref<MenuItem> ContextMenu::DoContextMenuByItem(ref<Wnd> wnd, Pixels x, Pixels y) {
	// If there are no items, always fail
	if(_menu->GetItemCount()==0) {
		return null;
	}
	
	if(wnd) {
		if(x<0 || y<0) {
			strong<Theme> theme = ThemeManager::GetTheme();
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
		ref<ContextPopupWnd> cpw = GC::Hold(new ContextPopupWnd(_menu, wnd->GetWindow()));
		return cpw->DoModal(wnd,x,y);
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

void ContextMenu::AddItem(ref<MenuItem> ci) {
	if(ci) {
		_menu->AddItem(strong<MenuItem>(ci));
	}
}

strong<Menu> ContextMenu::GetMenu() {
	return _menu;
}

/** if command == -1, then the item will be grayed out/disabled **/
void ContextMenu::AddItem(const std::wstring& name, int command, bool hilite, bool radiocheck) {
	strong<MenuItem> ci = GC::Hold(new MenuItem(name, command, hilite, radiocheck?MenuItem::Checked: MenuItem::NotChecked));
	_menu->AddItem(ci);
}

void ContextMenu::AddItem(const std::wstring& name, int command, bool hilite, MenuItem::CheckType checked) {
	strong<MenuItem> ci = GC::Hold(new MenuItem(name,command,hilite,checked));
	_menu->AddItem(ci);
}

void ContextMenu::AddSeparator(const std::wstring& text) {
	_menu->AddSeparator(text);
}

/** ContextPopupWnd **/
ContextPopupWnd::ContextPopupWnd(strong<Menu> menu, HWND parent): PopupWnd(parent,false), _firstMenu(menu), _mouseOver(-1), _mouseDown(-1), _checkedIcon(Icons::GetIconPath(Icons::IconChecked)), _radioCheckedIcon(Icons::GetIconPath(Icons::IconRadioChecked)), _subIcon(Icons::GetIconPath(Icons::IconSubMenu)) {
	SetWantMouseLeave(true);
	SetVerticallyScrollable(true);
	_openAnimation.SetLength(Time(300));
	_closeAnimation.SetLength(Time(200));
	_openAnimation.SetEase(Animation::EaseQuadratic);
}

ContextPopupWnd::~ContextPopupWnd() {
}

ref<MenuItem> ContextPopupWnd::DoModal(strong<Wnd> parent, Pixels x, Pixels y) {
	_menu.clear();
	EnterSubMenu(_firstMenu);
	strong<Menu> cm = _firstMenu;

	SetModal(true);
	PopupAt(x,y,parent);

	// Start modality
	ModalLoop::Result res = _loop.Enter(GetWindow(), false);

	SetModal(false);
	Show(false);

	if(res==ModalLoop::ResultSucceeded) {
		return _resultItem;
	}

	return null;
}

void ContextPopupWnd::EnterSubMenu(strong<Menu> cm) {
	_menu.push_front(cm);
	UpdateSize();

	// Start animation
	StartTimer(Time(50), 1);
	_openAnimation.Start();

	Repaint();
}

void ContextPopupWnd::LeaveSubMenu() {
	// This means leave without having selected an item
	_menu.pop_front();
	if(_menu.size()<1) {
		EndModal(null);
	}
	else {
		UpdateSize();
	}
	Repaint();
}

void ContextPopupWnd::UpdateSize() {
	// Calculate size
	strong<Menu> cm = GetCurrentMenu();
	strong<Theme> theme = ThemeManager::GetTheme();
	Pixels largestWidth = cm->GetLargestWidth(theme, theme->GetGUIFontBold());
	Pixels itemHeight = theme->GetMeasureInPixels(Theme::MeasureMenuItemHeight);
	Pixels width = max(theme->GetMeasureInPixels(Theme::MeasureMinimumContextMenuWidth),largestWidth+10)+itemHeight;

	// Set size
	unsigned int itemsShown = min((unsigned int)cm->GetItemCount(), ContextPopupWnd::KMaxItems);
	SetSize(width, Pixels(int(itemsShown)*itemHeight));
	
	// Set scroll info
	SetVerticalScrollInfo(Range<int>(0,int(cm->GetItemCount())*itemHeight), itemsShown*itemHeight + 1);
	SetVerticalPos(0);
}

void ContextPopupWnd::EndModal(ref<MenuItem> res) {
	_resultItem = res;

	if(Animation::IsAnimationsEnabled() && _resultItem == null) {
		_closeAnimation.Start();
		StartTimer(Time(50), 2);
	}
	else {
		_loop.End(ModalLoop::ResultSucceeded);
	}
}

int ContextPopupWnd::GetItemAt(Pixels y) {
	strong<Theme> theme = ThemeManager::GetTheme();
	return ((y+GetVerticalPos())/theme->GetMeasureInPixels(Theme::MeasureMenuItemHeight));
}

void ContextPopupWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	strong<Menu> cm = GetCurrentMenu();
	strong<Theme> theme = ThemeManager::GetTheme();
	Pixels leftHeaderSize = Pixels(_openAnimation.GetFraction()*(theme->GetMeasureInPixels(Theme::MeasureMenuItemHeight)/2));
	
	if(_menu.size() < 2 || x > leftHeaderSize) {
		// Mouse event handling for the menu items
		if(ev==MouseEventMove || ev==MouseEventLDown) {
			if(!_closeAnimation.IsAnimating()) {
				int idx = GetItemAt(y);
				if(idx>=0 && idx<int(cm->GetItemCount())) {
					_mouseOver = idx;
					if(ev==MouseEventLDown) {
						_mouseDown = idx;
					}
					else {
						_mouseDown = KMouseOverNothing;
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
			if(idx>=0 && idx<int(cm->GetItemCount())) {
				ref<MenuItem> ci = cm->GetItemByIndex(idx);
				if(ci) {
					OnSelectItem(ci);
				}
			}
		}
	}
	else {
		// Mouse event handling for the 'go back' button
		if(ev==MouseEventLDown) {
			_mouseDown = KMouseOverBackButton;
		}
		else if(ev==MouseEventMove) {
			_mouseOver = KMouseOverBackButton;
		}
		else if(ev==MouseEventLUp) {
			LeaveSubMenu();
		}
		Repaint();
	}

	if(ev==MouseEventLeave) {
		if(!_closeAnimation.IsAnimating()) {
			_mouseOver = -1;
			Repaint();
		}
	}
	PopupWnd::OnMouse(ev,x,y);
}

strong<Menu> ContextPopupWnd::GetCurrentMenu() {
	return _menu[0];
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
	strong<Menu> cm = GetCurrentMenu();

	if(!accelerator) {
		switch(k) {
			case KeyDown:
				if(down) {
					++_mouseOver;
					if(_mouseOver > int(cm->GetItemCount())-1) {
						_mouseOver = 0;
					}
					_mouseDown = KMouseOverNothing;
				}
				break;

			case KeyUp:
				if(down) {
					--_mouseOver;
					if(_mouseOver < 0) {
						_mouseOver = int(cm->GetItemCount())-1;
					}
					_mouseDown = KMouseOverNothing;
				}
				break;
			case KeyLeft:
				if(down) {
					LeaveSubMenu();
				}
				break;
			case KeyCharacter:
				if(ch==VK_SPACE) {
					if(down) {
						_mouseDown = _mouseOver;
					}
					else {
						if(_mouseDown >= 0 && _mouseDown < int(cm->GetItemCount())) {
							ref<MenuItem> ci = cm->GetItemByIndex(_mouseOver);
							if(ci) {
								OnSelectItem(ci);
							}
						}
					}
				}
				break;
		}
	}
	Repaint();
}

void ContextPopupWnd::OnSelectItem(strong<MenuItem> ci) {
	if(!ci->IsDisabled() && !ci->IsSeparator()) {
		ref<Menu> sub = ci->GetSubMenu();
		if(sub) {
			EnterSubMenu(sub);
		}
		else {
			EndModal(ci);
		}
	}
}

void ContextPopupWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	Area rc = GetClientArea();
	Area originalRc = rc;
	SolidBrush back(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&back, rc);
	theme->DrawInsetRectangle(g, rc);

	// If we're in a submenu, do a transition if we're animating
	if(_menu.size()>1) {
		// Draw left header (back button)
		Pixels leftHeaderSize = Pixels(_openAnimation.GetFraction()*(theme->GetMeasureInPixels(Theme::MeasureMenuItemHeight)/2));
		Area leftButtonRc(rc.GetLeft(), rc.GetTop(), leftHeaderSize, rc.GetHeight());
		LinearGradientBrush selected(PointF(0.0f, float(leftButtonRc.GetTop())), PointF(0.0f, float(leftButtonRc.GetBottom())), theme->GetColor(Theme::ColorTimeSelectionStart), theme->GetColor(Theme::ColorTimeSelectionEnd));
		LinearGradientBrush down(PointF(0.0f, float(leftButtonRc.GetTop())), PointF(0.0f, float(leftButtonRc.GetBottom())), theme->GetColor(Theme::ColorTimeSelectionEnd), theme->GetColor(Theme::ColorTimeSelectionStart));
	
		if(_mouseDown==KMouseOverBackButton) {
			leftButtonRc.Narrow(1,0,1,0);
			g.FillRectangle(&down, leftButtonRc);
		}
		else if(_mouseOver==KMouseOverBackButton) {
			leftButtonRc.Narrow(1,0,1,0);
			g.FillRectangle(&selected, leftButtonRc);
		}

		rc.Narrow(leftHeaderSize, 0, 0, 0);
		g.TranslateTransform(rc.GetWidth()-_openAnimation.GetFraction()*float(rc.GetWidth()), 0.0f);
	}

	SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
	g.FillRectangle(&disabled, rc);
	DrawMenuItems(g, theme, GetCurrentMenu(), rc);

	if(_menu.size()>1 && _openAnimation.IsAnimating()) {
		g.TranslateTransform(float(-rc.GetWidth()), 0.0f);
		strong<Menu> previous = _menu[1];
		DrawMenuItems(g, theme, previous, rc);
		g.FillRectangle(&disabled, rc);
	}

	// Draw border
	originalRc.Narrow(0,0,1,1);
	SolidBrush border(theme->GetColor(Theme::ColorActiveStart));
	Pen pn(&border, 1.0f);
	g.DrawRectangle(&pn, originalRc);
}

void ContextPopupWnd::DrawMenuItems(graphics::Graphics& g, strong<Theme> theme, strong<Menu> cm, const Area& rc) {
	Pixels y = -GetVerticalPos();
	Pixels itemHeight = theme->GetMeasureInPixels(Theme::MeasureMenuItemHeight);

	// Get us some colors
	SolidBrush text(theme->GetColor(Theme::ColorText));
	SolidBrush header(theme->GetColor(Theme::ColorActiveStart));
	SolidBrush link(theme->GetColor(Theme::ColorLink));
	SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
	SolidBrush closing(Theme::ChangeAlpha(theme->GetColor(Theme::ColorDisabledOverlay),_closeAnimation.GetFraction()));
	SolidBrush colorSeparator(theme->GetColor(Theme::ColorActiveEnd));
	Pen separator(&colorSeparator, 1.0f);
	LinearGradientBrush selected(PointF(0.0f, float(y)), PointF(0.0f, float(y+itemHeight)), theme->GetColor(Theme::ColorTimeSelectionStart), theme->GetColor(Theme::ColorTimeSelectionEnd));
	LinearGradientBrush down(PointF(0.0f, float(y)), PointF(0.0f, float(y+itemHeight)), theme->GetColor(Theme::ColorTimeSelectionEnd), theme->GetColor(Theme::ColorTimeSelectionStart));
	
	// Set up string format
	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);
	sf.SetLineAlignment(StringAlignmentCenter);
	sf.SetTrimming(StringTrimmingEllipsisPath);
	REAL tabs[] = {50, 50, 50};
	sf.SetTabStops(0, 3, tabs);
	Pixels indentSize = Pixels(itemHeight*0.619f);

	for(unsigned int n=0; n<cm->GetItemCount(); n++) {
		ref<MenuItem> item = cm->GetItemByIndex(n);
		if(item) {
			Area current = rc;
			current.SetY(y);
			current.SetHeight(itemHeight);

			Pixels indentRight = item->GetIndent()*indentSize;
			current.Narrow(indentRight,0,0,0);
			Area currentText = current;

			if(item->IsSeparator()) {
				const std::wstring& title = item->GetTitle();
				if(title.length()>0) {
					currentText.Narrow(itemHeight/2,2,2,2);
					g.DrawString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), currentText, &sf, &header);
					RectF lr;
					g.MeasureString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), currentText, &sf, &lr);
					current.Narrow((Pixels)lr.Width, 0,0,0);
				}

				current.Narrow(itemHeight,0,itemHeight,0);
				Pixels y = current.GetTop() + (current.GetBottom()-current.GetTop())/2;
				g.DrawLine(&separator, current.GetLeft(), y, current.GetRight(), y);
			}
			else {
				currentText.Narrow(itemHeight,2,2,2);

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

				// Draw submenu indicator
				if(item->GetSubMenu()) {
					Area subIconArea(current.GetRight()-16, current.GetTop(), 16, current.GetHeight());
					_subIcon.Paint(g, subIconArea);
				}
				// Draw hotkey label
				else if(item->HasHotkey()) {
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

				if(_closeAnimation.IsAnimating() && _resultItem != item) {
					g.FillRectangle(&closing, current);
				}
			}
		}

		y += itemHeight;
	}
}

void ContextPopupWnd::OnTimer(unsigned int id) {
	if(!_closeAnimation.IsAnimating() && _resultItem) {
		StopTimer(2);
		EndModal(null);
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

ref<Menu> MenuItem::GetSubMenu() {
	return null;
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

int MenuItem::GetCommandCode() const {
	return _command;
}

void MenuItem::SetCommandCode(int c) {
	_command = c;
}

/** SubMenuItem **/
SubMenuItem::SubMenuItem() {
}

SubMenuItem::SubMenuItem(const std::wstring& title, bool highlight, CheckType checked, const std::wstring& icon): MenuItem(title, 0, highlight, checked, icon) {
}

SubMenuItem::SubMenuItem(const std::wstring& title, bool highlight, CheckType checked, ref<Icon> icon): MenuItem(title, 0, highlight, checked, icon) {
}

SubMenuItem::~SubMenuItem() {
}

ref<Menu> SubMenuItem::GetSubMenu() {
	return ref<Menu>(this);
}

/** BasicMenu **/
BasicMenu::BasicMenu() {
}

BasicMenu::~BasicMenu() {
}

unsigned int BasicMenu::GetItemCount() const {
	return (unsigned int)_items.size();
}

ref<MenuItem> BasicMenu::GetItemByIndex(unsigned int idx) {
	try {
		return _items.at(idx);
	}
	catch(...) {
		return null;
	}
}

void BasicMenu::AddSeparator(const std::wstring& title) {
	strong<MenuItem> item = GC::Hold(new MenuItem());
	item->SetTitle(title);
	item->SetSeparator(true);
	_items.push_back(item);
}

void BasicMenu::AddItem(strong<MenuItem> ci) {
	_items.push_back(ci);

	if(ci->GetTitle().length() > _longestString.length()) {
		_longestString = ci->GetTitle();
	}
}

Pixels BasicMenu::GetLargestWidth(strong<Theme> theme, Font* fnt) const {
	 return theme->MeasureText(_longestString, fnt).GetWidth();
}