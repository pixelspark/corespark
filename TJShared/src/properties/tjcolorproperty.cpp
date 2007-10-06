#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

ColorChooserWnd::ColorChooserWnd(unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char* tred, unsigned char* tgreen, unsigned char* tblue): ChildWnd(L""), _colorsIcon(L"icons/shared/colors.png") {
	_red = red;
	_green = green;
	_blue = blue;
	_tred = tred;
	_tgreen = tgreen;
	_tblue = tblue;
}

ColorChooserWnd::~ColorChooserWnd() {
}

void ColorChooserWnd::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();
	SolidBrush bbr(theme->GetBackgroundColor());
	g.FillRectangle(&bbr, rc);

	rc.Narrow(1,1,1,1);
	SolidBrush cbr(Color(*_red, *_green, *_blue));
	g.FillRectangle(&cbr, rc);

	Area iconArea(rc.GetRight()-20, rc.GetTop(), 16, 16);
	g.DrawImage(_colorsIcon, iconArea);
}

LRESULT ColorChooserWnd::Message(UINT msg, WPARAM wp,LPARAM lp) {
	if(msg==WM_LBUTTONUP) {
		COLORREF g_rgbBackground = RGB(*_red, *_green, *_blue);
		COLORREF g_rgbCustom[16] = {0};

		CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};

		cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR;
		cc.hwndOwner = GetWindow();
		cc.rgbResult = g_rgbBackground;
		cc.lpCustColors = g_rgbCustom;

		if(ChooseColor(&cc)) {
			*_red = GetRValue(cc.rgbResult);
			*_green = GetGValue(cc.rgbResult);
			*_blue = GetBValue(cc.rgbResult);

			if(_tred!=0) {
				*_tred = *_red;
			}

			if(_tgreen!=0) {
				*_tgreen = *_green;
			}

			if(_tblue!=0) {
				*_tblue = *_blue;
			}
		}

		Repaint();
	}
	return ChildWnd::Message(msg,wp,lp);
}

ColorProperty::ColorProperty(std::wstring name,unsigned char* red, unsigned char* green, unsigned char* blue,unsigned char* tred, unsigned char* tgreen, unsigned char* tblue): Property(name), _red(red), _green(green), _blue(blue), _wnd(0), _tRed(tred), _tGreen(tgreen), _tBlue(tblue)  {
}

ColorProperty::~ColorProperty() {
}

void ColorProperty::Changed() {
}

HWND ColorProperty::Create(HWND parent) {
	if(!_wnd) {
		_wnd = GC::Hold(new ColorChooserWnd(_red, _green, _blue, _tRed, _tGreen, _tBlue));
		SetParent(_wnd->GetWindow(), parent);
		_wnd->SetStyle(WS_CHILD);
	}
	
	return _wnd->GetWindow();
}

std::wstring ColorProperty::GetValue() {
	return L"";
}

HWND ColorProperty::GetWindow() {
	return _wnd->GetWindow();
}

void ColorProperty::Update() {
	// kleurtje van de knop updaten
}