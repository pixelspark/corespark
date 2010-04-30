/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #include "../include/tjsharedui.h"
#include <commctrl.h>
using namespace tj::shared;
using namespace tj::shared::graphics;

// declared and used in tjui.cpp, but shouldn't be public
LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

LRESULT CALLBACK EditWndSubclassProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,UINT_PTR uIdSubclass,DWORD_PTR dwRefData) {
	// This is the same behaviour as in Wnd::Message
	if(uMsg==WM_KEYDOWN && LOWORD(wParam)==VK_TAB) {
		HWND root = GetAncestor(hWnd, GA_ROOT);
		HWND next = GetNextDlgTabItem(root, hWnd, !Wnd::IsKeyDown(KeyShift));
		SetFocus(next);
		return 0;
	}
	else if((uMsg==WM_CHAR || uMsg==WM_KEYUP)) {
		if(LOWORD(wParam)==VK_TAB) {
			// consume
			return 0;
		}
	}
	else if((uMsg==WM_KEYDOWN || uMsg==WM_KEYUP || uMsg==WM_CHAR) && (wParam==VK_RETURN || wParam==VK_DOWN)) {
		if(uMsg==WM_KEYDOWN) {
			HWND parent = ::GetParent(hWnd);
			SendMessage(parent, WM_PARENTNOTIFY, WM_KEYDOWN, (LPARAM)wParam);
		}

		// consume; EditWnd will send an EditingCommit notification
		// The edit control doesn't do anything with the return key press, but it does
		// play a system beep to indicate that the key is disabled. This of course is
		// undesired in applications like TJingle.
		return 0; 
	}
	else if(uMsg==WM_SETFOCUS) {
		// Send message to parent (this is useful for PropertyGridWnd, for example
		HWND parent = ::GetParent(hWnd);
		SendMessage(parent, WM_PARENTNOTIFY, WM_SETFOCUS, 0);
	}
	else if(uMsg==WM_KILLFOCUS) {
		// Send message to parent (this is useful for PropertyGridWnd, for example
		HWND parent = ::GetParent(hWnd);
		SendMessage(parent, WM_PARENTNOTIFY, WM_KILLFOCUS, 0);
	}
	
	LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
	if(uMsg==WM_KEYUP && LOWORD(wParam)==VK_RETURN) {
		LONG style = GetWindowLong(hWnd, GWL_STYLE);
		if((style & ES_WANTRETURN)!=0) {
			return 1; // let the ModalLoop know we handled this message, preventing it from ending the modal loop
		}
	}

	return res;
}

EditWnd::EditWnd(bool multiline): ChildWnd(false), _backBrush(0) {
	Create(multiline);
}

EditWnd::EditWnd(bool ml, bool db): ChildWnd(db), _backBrush(0) {
	Create(ml);
}

void EditWnd::Create(bool multiline) {
	SetStyle(WS_CLIPCHILDREN);
	SetStyleEx(WS_EX_CONTROLPARENT);

	long flags = WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL;
	if(multiline) {
		flags |= ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL;
	}

	_ctrl = CreateWindowEx(0, L"EDIT", L"", flags, 0, 0, 10, 10, GetWindow(), 0, GetModuleHandle(NULL), 0);
	_font = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
	
	SetWindowSubclass(_ctrl, EditWndSubclassProc, 1, 0);
	SendMessage(_ctrl, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);
	Layout();
}

EditWnd::~EditWnd() {
	RemoveWindowSubclass(_ctrl, EditWndSubclassProc, 1);
	DestroyWindow(_ctrl);
	DeleteObject(_font);
	DeleteObject(_backBrush);
}

void EditWnd::UpdateColor() {
	strong<Theme> theme = ThemeManager::GetTheme();
	Color nb = theme->GetColor(Theme::ColorEditBackground);
	if(nb.GetValue()!=_back.GetValue()||_backBrush==0) {
		_back = nb;
		if(_backBrush!=0) DeleteObject(_backBrush);
		_backBrush = CreateSolidBrush(RGB(_back.GetRed(), _back.GetGreen(), _back.GetBlue()));
	}
}

void EditWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
}

void EditWnd::Show(bool s) {
	if(s) {
		ShowWindow(_ctrl, SW_SHOW);
	}
	ChildWnd::Show(s);
}

void EditWnd::Layout() {
	Area rc = GetClientArea();
	strong<Theme> theme = ThemeManager::GetTheme();
	rc.MultiplyCeil(theme->GetDPIScaleFactor(), theme->GetDPIScaleFactor());
	SetWindowPos(_ctrl, 0L, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight(), SWP_NOZORDER);
}

void EditWnd::SetReadOnly(bool r) {
	SendMessage(_ctrl, EM_SETREADONLY, (WPARAM)(BOOL)r, 0);
}

void EditWnd::SetBorder(bool t) {
	long s = GetWindowLong(_ctrl, GWL_EXSTYLE);
	if(t) {
		s |= WS_EX_CLIENTEDGE;
	}
	else {
		s &= (~WS_EX_CLIENTEDGE);
	}
	SetWindowLong(_ctrl, GWL_EXSTYLE, s);
	Repaint();
}

LRESULT EditWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CTLCOLOREDIT) {
		UpdateColor();
		/// SetBkMode(TRANSPARENT) doesn't seem to work with multiline edit controls
		///SetBkMode((HDC)wp, TRANSPARENT);

		Color back = ThemeManager::GetTheme()->GetColor(Theme::ColorBackground);
		SetBkColor((HDC)wp, RGB(back.GetRed(), back.GetGreen(), back.GetBlue()));

		Color text = ThemeManager::GetTheme()->GetColor(Theme::ColorText);
		SetTextColor((HDC)wp, RGB(text.GetRed(),text.GetGreen(),text.GetBlue()));
		
		return (LRESULT)(HBRUSH)_backBrush;
		
	}
	else if(msg==WM_COMMAND) {
		if(HIWORD(wp)==EN_CHANGE) {
			EventEditing.Fire(ref<Object>(this), EditingNotification(EditingTextChanged));
			UpdateWindow(_ctrl);
		}
		else {
			HWND parent = ::GetParent(GetWindow());
			if(parent!=0) {
				return SendMessage(parent, msg, wp, lp);
			}
		}
	}
	else if(msg==WM_PARENTNOTIFY) {
		if(wp==WM_SETFOCUS) {
			HWND parent = ::GetParent(GetWindow());
			SendMessage(parent, WM_PARENTNOTIFY, WM_SETFOCUS, 0);
			EventEditing.Fire(ref<Object>(this), EditingNotification(EditingStarted));
		}
		else if(wp==WM_KILLFOCUS) {
			HWND parent = ::GetParent(GetWindow());
			SendMessage(parent, WM_PARENTNOTIFY, WM_KILLFOCUS, 0);
			EventEditing.Fire(ref<Object>(this), EditingNotification(EditingEnded));
		}
		else if(wp==WM_KEYDOWN) {
			if(lp==VK_DOWN || lp==VK_RETURN) {
				EventEditing.Fire(ref<Object>(this), EditingNotification(EditingCommit));
			}
		}
	}
	return ChildWnd::Message(msg,wp,lp);
}

void EditWnd::OnSize(const Area& ns) {
	Layout();
	Repaint();
}

void EditWnd::Focus() {
	OnFocus(true);
}

void EditWnd::OnFocus(bool f) {
	if(f) {
		SetFocus(_ctrl);
	}
}

bool EditWnd::HasFocus(bool childrenToo) const {
	return GetFocus() == _ctrl;
}

void EditWnd::SetCue(const std::wstring& cue) {
	SendMessage(_ctrl, EM_SETCUEBANNER, 0, (LPARAM)cue.c_str());
}

void EditWnd::SetText(const std::wstring& x) {
	SetWindowText(_ctrl, x.c_str());
}

std::wstring EditWnd::GetText() {
	int n = GetWindowTextLength(_ctrl);
	wchar_t* buffer = new wchar_t[n+2];
	GetWindowText(_ctrl, buffer, n+1);
	std::wstring txt(buffer);
	delete[] buffer;
	return txt;
}

EditWnd::EditingNotification::EditingNotification(EditingType type): _type(type) {
}

EditWnd::EditingType EditWnd::EditingNotification::GetType() const {
	return _type;
}

/** SuggestionEditWnd **/
SuggestionEditWnd::SuggestionEditWnd(bool ml): EditWnd(ml, true), _arrowIcon(Icons::GetIconPath(Icons::IconDownArrow)), _sm(SuggestionModeReplace) {
	SetWantMouseLeave(true);
}

SuggestionEditWnd::~SuggestionEditWnd() {
}

void SuggestionEditWnd::SetSuggestionMode(SuggestionMode sm) {
	_sm = sm;
}

SuggestionEditWnd::SuggestionMode SuggestionEditWnd::GetSuggestionMode() const {
	return _sm;
}

ref<MenuItem> SuggestionEditWnd::DoSuggestionMenu() {
	if(_cm) {
		Area rc = GetClientArea();
		ref<MenuItem> mi = _cm->DoContextMenuByItem(this, rc.GetLeft(), rc.GetBottom());
		if(mi && mi.IsCastableTo<SuggestionMenuItem>()) {
			ref<SuggestionMenuItem> smi = mi;
			const std::wstring& val = smi->GetSuggestionValue();
			
			switch(_sm) {
				case SuggestionModeReplace:
					SetText(val);
					break;

				case SuggestionModeInsert:
					// Somehow insert the text, for now: append
					SetText(GetText()+val);
					break;
			}

			EventEditing.Fire(this, EditingNotification(EditingTextChanged));
		}
		
		return mi;
	}
	return null;
}

strong<Menu> SuggestionEditWnd::GetSuggestionMenu() {
	if(!_cm) {
		_cm = GC::Hold(new ContextMenu());
	}

	return _cm->GetMenu();
}

void SuggestionEditWnd::Layout() {
	Area rc = GetClientArea();
	rc.Narrow(0,0,16,0);
	strong<Theme> theme = ThemeManager::GetTheme();
	rc.MultiplyCeil(theme->GetDPIScaleFactor(), theme->GetDPIScaleFactor());
	SetWindowPos(_ctrl, 0L, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight(), SWP_NOZORDER);
}

void SuggestionEditWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	// Draw background
	Area rc = GetClientArea();
	graphics::SolidBrush back(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&back, rc);
	theme->DrawInsetRectangleLight(g, rc);

	graphics::SolidBrush borderBrush(theme->GetColor(Theme::ColorActiveStart));
	Area borderArea = rc;
	borderArea.Narrow(0,0,1,1);
	graphics::Pen borderPen(&borderBrush, 1.0f);
	g.DrawRectangle(&borderPen, borderArea);
	
	Area buttonArea(rc.GetRight()-18, rc.GetTop(), 18, rc.GetHeight());

	graphics::LinearGradientBrush buttonBr(graphics::PointF(0.0f, float(rc.GetTop()-1)), graphics::PointF(0.0f, float(rc.GetBottom()+1)), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
	g.FillRectangle(&buttonBr, buttonArea);

	if(!IsMouseOver()) {
		buttonArea.Narrow(3,1,1,1);
		graphics::SolidBrush disabledBr(theme->GetColor(Theme::ColorDisabledOverlay));
		g.FillRectangle(&disabledBr, buttonArea);
	}

	// Draw icon to the right
	Area iconArea(rc.GetRight()-16, rc.GetTop(), 16, 16);
	g.DrawImage(_arrowIcon, iconArea);

	// If there are no items, we are disabled
	if(!_cm || _cm->GetMenu()->GetItemCount() < 1) {
		Area drc = rc;
		drc.Narrow(1,1,1,1);
		graphics::SolidBrush dbr(theme->GetColor(Theme::ColorDisabledOverlay));
		g.FillRectangle(&dbr, drc);
	}
}	

void SuggestionEditWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventMove) {
		SetWantMouseLeave(true);
		Repaint();
	}
	else if(ev==MouseEventLeave) {
		Repaint();
	}
	else if(ev==MouseEventLDown) {
		if(x>16) {
			// Do context menu
			DoSuggestionMenu();
		}
		
	}
	EditWnd::OnMouse(ev,x,y);
}

SuggestionMenuItem::SuggestionMenuItem(const std::wstring& value, const std::wstring& friendly, bool highlight, MenuItem::CheckType checked, const std::wstring& icon, const std::wstring& hotkey): MenuItem(friendly, 0, highlight, checked, icon, hotkey), _value(value) {
}

SuggestionMenuItem::~SuggestionMenuItem() {
}

const std::wstring& SuggestionMenuItem::GetSuggestionValue() const {
	return _value;
}