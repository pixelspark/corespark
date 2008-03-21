#include "../../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

ChildWnd::ChildWnd(const wchar_t* title, bool wantDoubleClick, bool useDB): Wnd(title, 0L, wantDoubleClick?TJ_DEFAULT_CLASS_NAME:(L"TjWndClassNdbl"), useDB) {
	SetWindowLong(GetWindow(), GWL_STYLE, WS_CHILD);
	SetWindowLong(GetWindow(), GWL_EXSTYLE, 0);

	SetStyle(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_TABSTOP);
}

LRESULT ChildWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_KEYDOWN && wp==VK_TAB) {
		HWND focused = GetFocus();
		HWND owner = GetAncestor(GetWindow(), GA_ROOT);
		HWND next = GetNextDlgTabItem(owner, focused, FALSE);
		SetFocus(next);
	}

	return Wnd::Message(msg,wp,lp);
}

CheckboxWnd::CheckboxWnd(): ChildWnd(L""), _checkedIcon(Icons::GetIconPath(Icons::IconChecked)), _checked(false) {
}

CheckboxWnd::~CheckboxWnd() {
}

bool CheckboxWnd::IsChecked() const {
	return _checked;
}

void CheckboxWnd::SetChecked(bool t) {
	_checked = t;
	Repaint();
}

void CheckboxWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		Focus();
		SetChecked(!_checked);
		Repaint();
	}
	else if(ev==MouseEventMove) {
		SetWantMouseLeave(true);
		Repaint();
	}
	else if(ev==MouseEventLeave) {
		Repaint();
	}
}

void CheckboxWnd::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
	SolidBrush back(theme->GetBackgroundColor());
	Area rc = GetClientArea();
	g.FillRectangle(&back, rc);
	bool down = IsKeyDown(KeyMouseLeft);

	Pixels margin = (rc.GetHeight()-16)/2;
	LinearGradientBrush lbr(PointF(0.0f, (float)margin), PointF(0.0f, float(margin+17)), theme->GetActiveStartColor(), theme->GetActiveEndColor());
	Pen borderPen(&lbr, HasFocus()?2.0f:1.0f);

	Area rect(rc.GetLeft()+margin, rc.GetTop()+margin, 16, 16);
	Area borderRect = rect;
	borderRect.Narrow(2,2,2,2);

	if(IsMouseOver() || down) {
		LinearGradientBrush focusBr(PointF(0.0f, float(rc.GetTop())), PointF(0.0f, (float)rc.GetBottom()), down?theme->GetTimeSelectionColorEnd():theme->GetTimeSelectionColorStart(), down?theme->GetTimeSelectionColorStart():theme->GetTimeSelectionColorEnd());
		g.FillRectangle(&focusBr, rect);
		SolidBrush disabled(theme->GetDisabledOverlayColor());
		g.FillRectangle(&disabled, rect);
	}

	g.DrawRectangle(&borderPen, borderRect);
	if(_checked) {
		g.DrawImage(_checkedIcon, rect);
	}
}

void CheckboxWnd::OnSize(const Area& ns) {
	Repaint();
}

void CheckboxWnd::OnFocus(bool f) {
	Repaint();
}

void CheckboxWnd::OnKey(Key k, wchar_t ch, bool down) {
	if(down && (k==KeyLeft || k==KeyRight)) {
		SetChecked(!_checked);
	}
}