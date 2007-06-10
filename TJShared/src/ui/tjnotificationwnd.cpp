#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

const int NotificationWnd::DefaultNotificationHeight = 30;
const int NotificationWnd::DefaultNotificationMargin = 10;
const int NotificationWnd::DefaultNotificationTimeout = 5000;

NotificationWnd::NotificationWnd(const std::wstring& text, std::wstring icon, int time, int h, RootWnd* parent) : Wnd(text.c_str(), (parent!=0)?parent->GetWindow():0L, TJ_DROPSHADOW_CLASS_NAME) {
	assert(parent!=0);
	SetStyle(WS_POPUP);
	UnsetStyle(WS_CAPTION|WS_BORDER);
	SetStyleEx(WS_EX_LAYERED);
	_text = text;
	_index = h;
	std::wstring fn = ResourceManager::Instance()->Get(icon);
	_icon = Bitmap::FromFile(fn.c_str());
	_root = parent;

	if(time==NotificationTimeoutDefault) {
		time = DefaultNotificationTimeout;
	}

	RECT rc;
	GetClientRect(parent->GetWindow(), &rc);

	if(time>0) {
		SetTimer(GetWindow(), 1337, time, 0L);
	}
	SetWindowPos(GetWindow(), 0L, DefaultNotificationMargin,DefaultNotificationMargin+h*(DefaultNotificationMargin+DefaultNotificationHeight), rc.right-rc.left-(2*DefaultNotificationMargin), DefaultNotificationHeight, SWP_NOZORDER);
	SetWindowPos(GetWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE | SWP_NOSIZE);
	Show(true);
}

NotificationWnd::~NotificationWnd() {
	delete _icon;
}

void NotificationWnd::Show(bool t) {
	Wnd::Show(t);
}

int NotificationWnd::GetIndex() {
	return _index;
}

void NotificationWnd::Paint(Gdiplus::Graphics& g) {
	ref<Theme> theme = ThemeManager::GetTheme();
	Area rect = GetClientArea();
	//SolidBrush r(theme->ChangeAlpha(theme->GetBackgroundColor(),100));
	//LinearGradientBrush r(PointF(0.0f, -1.0f), PointF(0.0f, float(rect.GetHeight())+1.0f), theme->GetActiveStartColor(), theme->GetActiveEndColor());
	SolidBrush r(theme->GetBackgroundColor());
	rect.Narrow(0,0,1,1);
	g.FillRectangle(&r, rect);

	LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(rect.GetHeight())), theme->GetActiveStartColor(), theme->GetActiveEndColor());
	Pen lbp(&lbr, 1.0f);
	g.DrawRectangle(&lbp, rect);

	Area stringRect = rect;
	stringRect.Narrow(DefaultNotificationHeight, DefaultNotificationMargin/2, 0, 0);
	StringFormat sf;
	sf.SetLineAlignment(StringAlignmentCenter);
	
	SolidBrush textBrush(theme->GetTextColor());
	g.DrawString(_text.c_str(), (unsigned int)_text.length(), theme->GetGUIFontBold(), stringRect, &sf, &textBrush);

	g.DrawImage(_icon, RectF(4.0f, 4.0f, 22.0f, 22.0f));
}

LRESULT NotificationWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_LBUTTONDOWN||msg==WM_TIMER) {
		Show(false);
		ReplyMessage(0);
		_root->RemoveNotification(this);
		return 0;
	}
	else if(msg==WM_DESTROY) {
	}
	else if(msg==WM_ACTIVATE) {
		Repaint();
	}
	return Wnd::Message(msg, wp, lp);
}