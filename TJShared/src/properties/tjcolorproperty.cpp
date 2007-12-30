#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

/* ColorWheel */
ColorWheel::ColorWheel() {
	_bitmap = 0;
	_w = 0;
	_h = 0;
}

ColorWheel::~ColorWheel() {
	delete _bitmap;
}

void ColorWheel::SetSize(Pixels x, Pixels y) {
	_w = x;
	_h = y;
	delete _bitmap;
	_bitmap = 0;
}

void ColorWheel::PaintMarker(Gdiplus::Graphics& g, ref<Theme> theme, Pixels offx, Pixels offy, double h, double s) {
	Pixels centerX = (_w/2)+offx;
	Pixels centerY = (_h/2)+offy;
	float radius = float(min(_w,_h))/2.0f;
	
	Pixels x = Pixels(centerX + cos(h*2.0*3.14159)*radius*s);
	Pixels y = Pixels(centerY + sin(h*2.0*3.14159)*radius*s);
	
	SolidBrush black(Color::Black);
	g.FillEllipse(&black, RectF(float(x-2.5f), float(y-2.5f), 5.0f, 5.0f));
}

void ColorWheel::Paint(Gdiplus::Graphics& g, ref<Theme> theme, Pixels offx, Pixels offy) {	
	if(_bitmap==0) {
		float scale = theme->GetDPIScaleFactor();
		int bitmapWidth = int(scale*_w);
		int bitmapHeight = int(scale*_h);
		_bitmap = new Bitmap(bitmapWidth, bitmapHeight, &g);

		// Draw background
		SolidBrush back(Color(0,255,255,255)); // transparent
		Graphics bg(_bitmap);
		bg.SetSmoothingMode(SmoothingModeHighQuality);
		bg.SetCompositingQuality(CompositingQualityHighQuality);
		bg.FillRectangle(&back, Rect(0,0,bitmapWidth, bitmapHeight));

		// Calculate center
		float centerX = float(bitmapWidth)/2.0f;
		float centerY = float(bitmapHeight)/2.0f;
		float radius = min(bitmapWidth,bitmapHeight)/2.0f;

		// Create path
		static const int PointCount = 64;
		PointF points[PointCount];
		Color colors[PointCount];

		for(int a=0;a<PointCount;a++) {
			float deg = (float(a)/float(PointCount))*2.0f*3.14159f;
			points[a].X = cos(deg)*radius + centerX;
			points[a].Y = sin(deg)*radius + centerY;
			colors[a] = ColorSpaces::HSVToRGB(double(a)/double(PointCount), 1.0, 1.0);
		}

		PathGradientBrush pgb(points, PointCount);
		pgb.SetCenterColor(Color::White);
		pgb.SetCenterPoint(PointF(centerX, centerY));

		int numColors = PointCount;
		pgb.SetSurroundColors(colors,&numColors);
		bg.FillEllipse(&pgb, centerX-radius, centerY-radius, radius*2.0f, radius*2.0f);
	}
	
	g.DrawImage(_bitmap, RectF(float(offx), float(offy), float(_w), float(_h)));
}

Gdiplus::Color ColorWheel::GetColorAt(Pixels x, Pixels y, double brightness) {
	return ColorSpaces::HSVToRGB(GetHueAt(x,y), GetSaturationAt(x,y), brightness);	
}

double ColorWheel::GetHueAt(Pixels x, Pixels y) {
	Pixels centerX = _w/2;
	Pixels centerY = _h/2;
	return min(2.0*3.14159,fmod((atan2(double(y-centerY),double(x-centerX))+2.0*3.14159)/(2.0*3.14159), 2.0*3.14159));
}

double ColorWheel::GetSaturationAt(Pixels x, Pixels y) {
	Pixels centerX = _w/2;
	Pixels centerY = _h/2;

	float radius = min(_w,_h)/2.0f;
	double dx = x-centerX;
	double dy = y-centerY;

	return min(1.0,sqrt(double((dx*dx) + (dy*dy)))/radius);
}

/* ColorPopupWnd */
ColorPopupWnd::ColorPopupWnd(): _hue(0.0f), _sat(0.0f), _val(1.0) {
	SetSize(300,160);
	_wheel.SetSize(150,150);
	_brightness = GC::Hold(new SliderWnd(L""));
	_brightness->SetValue(1.0f);
	Add(_brightness);
	Layout();
}

ColorPopupWnd::~ColorPopupWnd() {
}

void ColorPopupWnd::OnCreated() {
	_brightness->SetListener(this);
}

CMYKColor::CMYKColor(double c, double m, double y, double k): _c(c), _m(m), _y(y), _k(k) {
}

HSVColor::HSVColor(double h, double s, double v): _h(h), _s(s), _v(v) {
}

void ColorPopupWnd::Notify(Wnd* source, Notification evt) {
	_val = _brightness->GetValue();
	Update();
}

void ColorPopupWnd::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();
	SolidBrush back(theme->GetBackgroundColor());
	g.FillRectangle(&back, rc);

	_wheel.Paint(g,theme,KWheelMargin, KWheelMargin);
	_wheel.PaintMarker(g,theme,KWheelMargin,KWheelMargin,_hue,_sat);

	Pixels left = 150+KWheelMargin;
	SolidBrush color(_color);
	g.FillRectangle(&color, RectF(float(left), float(KWheelMargin), rc.GetWidth()-float(left)-2.0f*float(KWheelMargin), 24.0f));

	// text
	Pixels textLeft = 150+KWheelMargin*2+(_brightness->GetClientArea().GetWidth());
	Pixels textTop = 2*KWheelMargin+24;
	
	std::wstring rgb = L"RGB: "+Stringify(_color.GetR())+L", "+Stringify(_color.GetG())+L", "+Stringify(_color.GetB());
	CMYKColor cmyk = ColorSpaces::RGBToCMYK(_color.GetR()/255.0, _color.GetG()/255.0, _color.GetB()/255.0);
	
	std::wostringstream cmos;
	cmos << L"CMYK: " << int(cmyk._c*100) << L"%, " << int(cmyk._m*100) << L"%, " << int(cmyk._y*100) << L"%, " << int(cmyk._k*100) << L"%, ";
	std::wstring cms = cmos.str();

	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);
	SolidBrush textBr(theme->GetTextColor());
	g.DrawString(rgb.c_str(), (int)rgb.length(), theme->GetGUIFont(), Area(textLeft, textTop, rc.GetWidth()-textLeft-KWheelMargin, 24), &sf, &textBr);
	g.DrawString(cms.c_str(), (int)cms.length(), theme->GetGUIFont(), Area(textLeft, textTop+24, rc.GetWidth()-textLeft-KWheelMargin, 24), &sf, &textBr);

	// Draw border
	rc.Narrow(0,0,1,1);
	SolidBrush border(theme->GetActiveStartColor());
	Pen pn(&border, 1.0f);
	g.DrawRectangle(&pn, rc);
}

void ColorPopupWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventMove && ISVKKEYDOWN(VK_LBUTTON) && x<155) {
		Pixels cx = x-KWheelMargin;
		Pixels cy = y-KWheelMargin;

		_hue = float(_wheel.GetHueAt(cx,cy));
		_sat = float(_wheel.GetSaturationAt(cx,cy));
		Update();
	}
}

void ColorPopupWnd::SetListener(ref<Listener> listener) {
	_myListener = listener;
}

void ColorPopupWnd::Update() {
	_brightness->SetValue(_val,false);
	_color = ColorSpaces::HSVToRGB(_hue,_sat,_val);

	ref<Listener> listener = _myListener;
	if(listener) {
		listener->Notify(dynamic_cast<Wnd*>(this), NotificationUpdate);
	}
	Repaint();
}

void ColorPopupWnd::OnSize(const Area& ns) {
}

void ColorPopupWnd::Layout() {
	Area rc = GetClientArea();
	rc.Narrow(150+KWheelMargin, 35, KWheelMargin, KWheelMargin);
	_brightness->Fill(LayoutLeft, rc);
}

Color ColorPopupWnd::GetColor() {
	return _color;
}

void ColorPopupWnd::SetColor(double r, double g, double b) {
	HSVColor hsv = ColorSpaces::RGBToHSV(r,g,b);
	_hue = float(hsv._h);
	_sat = float(hsv._s);
	_val = float(hsv._v);
	_brightness->SetValue(_val);
	Update();
}

/* ColorChooserWnd */
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

void ColorChooserWnd::Notify(Wnd* source, Notification not) {
	if(not==NotificationUpdate && _cpw) {
		Color col = _cpw->GetColor();
		*_red = col.GetRed();
		*_green = col.GetGreen();
		*_blue = col.GetBlue();
		if(_tred!=0) *_tred = *_red;
		if(_tgreen!=0) *_tgreen = *_green;
		if(_tblue!=0) *_tblue = *_blue;
		Repaint();
	}
}

void ColorChooserWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLUp) {
		if(!_cpw) {
			_cpw = GC::Hold(new ColorPopupWnd());
			_cpw->SetListener(this);
		}
		POINT pt;

		ref<Theme> theme = ThemeManager::GetTheme();
		Area rc = GetClientArea();
		pt.x = 0;
		pt.y = rc.GetHeight()+1;
		ClientToScreen(GetWindow(), &pt);
		SetWindowPos(_cpw->GetWindow(), 0, pt.x, pt.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
		_cpw->SetColor(double(*_red)/255.0, double(*_green)/255.0, double(*_blue)/255.0);
		_cpw->Show(true);
	}
	else if(ev==MouseEventRDown) {
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