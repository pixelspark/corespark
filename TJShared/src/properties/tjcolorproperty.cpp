#include "../../include/properties/tjproperties.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

#ifdef _WIN32
	// We provide a way to use the Windows color chooser
	#include <commdlg.h>
#endif

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

void ColorWheel::PaintMarker(graphics::Graphics& g, ref<Theme> theme, Pixels offx, Pixels offy, double h, double s) {
	Pixels centerX = (_w/2)+offx;
	Pixels centerY = (_h/2)+offy;
	float radius = float(min(_w,_h))/2.0f;
	
	Pixels x = Pixels(centerX + cos(h*2.0*3.14159)*radius*s);
	Pixels y = Pixels(centerY + sin(h*2.0*3.14159)*radius*s);
	
	SolidBrush black(Color(0,0,0));
	g.FillEllipse(&black, RectF(float(x-2.5f), float(y-2.5f), 5.0f, 5.0f));
}

void ColorWheel::Paint(graphics::Graphics& g, ref<Theme> theme, Pixels offx, Pixels offy) {	
	if(_bitmap==0) {
		float scale = theme->GetDPIScaleFactor();
		int bitmapWidth = int(scale*_w);
		int bitmapHeight = int(scale*_h);
		_bitmap = new Bitmap(bitmapWidth, bitmapHeight, &g);

		// Draw background
		SolidBrush back(Color(0,255,255,255)); // transparent
		Graphics bg(_bitmap);
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
			points[a]._x = cos(deg)*radius + centerX;
			points[a]._y = sin(deg)*radius + centerY;
			colors[a] = ColorSpaces::HSVToRGB(double(a)/double(PointCount), 1.0, 1.0);
		}

		PathGradientBrush pgb(points, PointCount);
		pgb.SetCenterColor(Color(1.0, 1.0, 1.0));
		pgb.SetCenterPoint(PointF(centerX, centerY));

		pgb.SetSurroundColors(colors, PointCount);
		bg.FillEllipse(&pgb, RectF(centerX-radius, centerY-radius, radius*2.0f, radius*2.0f));
	}
	
	g.DrawImage(_bitmap, RectF(float(offx), float(offy), float(_w), float(_h)));
}

RGBColor ColorWheel::GetColorAt(Pixels x, Pixels y, double brightness) {
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

RGBColor ColorPopupWnd::_favColors[ColorPopupWnd::KFavouriteColorCount] = {
	RGBColor(0.0f, 0.0f, 0.0f), RGBColor(1.0f, 1.0f, 1.0f), RGBColor(1.0f, 0.0f, 0.0f),
	RGBColor(0.0f, 1.0f, 0.0f), RGBColor(0.0f, 0.0f, 1.0f), RGBColor(1.0f, 1.0f, 0.0f),
	RGBColor(0.0f, 1.0f, 1.0f), RGBColor(1.0f, 0.0f, 1.0f), RGBColor(0.0f, 0.0f, 0.5f),
	RGBColor(0.5f, 0.5f, 0.5f),
};

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
	_brightness->EventChanged.AddListener(ref<Listener<SliderWnd::NotificationChanged> >(this));
}

CMYKColor::CMYKColor(double c, double m, double y, double k): _c(c), _m(m), _y(y), _k(k) {
}

HSVColor::HSVColor(double h, double s, double v): _h(h), _s(s), _v(v) {
}

void ColorPopupWnd::Notify(ref<Object> source, const SliderWnd::NotificationChanged& evt) {
	_val = _brightness->GetValue();
	Update();
}

void ColorPopupWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	Area rc = GetClientArea();
	SolidBrush back(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&back, rc);

	_wheel.Paint(g,theme,KWheelMargin, KWheelMargin);
	_wheel.PaintMarker(g,theme,KWheelMargin,KWheelMargin,_hue,_sat);

	Pixels left = 150+KWheelMargin;
	SolidBrush color(_color);
	Pixels width = Pixels(rc.GetWidth()-float(left)-2.0f*float(KWheelMargin));
	g.FillRectangle(&color, RectF(float(left), float(KWheelMargin), float(width), 24.0f));

	// Often used colors
	Pixels w = width/KFavouriteColorCount;

	for(int a=0;a<KFavouriteColorCount;a++) {
		const RGBColor& col = _favColors[a];
		Color gcol((BYTE)(col._r*255), (BYTE)(col._g*255), (BYTE)(col._b*255));
		SolidBrush brush(gcol);
		g.FillRectangle(&brush, RectF(float(left+w*a+1), float(KWheelMargin)+24.0f+1.0f, float(w-2), 12.0f-2.0f));
	}

	// text
	Pixels textLeft = 150+KWheelMargin*2+(_brightness->GetClientArea().GetWidth());
	Pixels textTop = 2*KWheelMargin+24+24;
	
	std::wstring rgb = L"RGB: "+Stringify(int(_color._r*255.0))+L", "+Stringify(int(_color._g*255.0))+L", "+Stringify(int(_color._b*255.0));
	CMYKColor cmyk = ColorSpaces::RGBToCMYK(_color._r, _color._g, _color._b);
	
	std::wostringstream cmos;
	cmos << L"CMYK: " << int(cmyk._c*100) << L"%, " << int(cmyk._m*100) << L"%, " << int(cmyk._y*100) << L"%, " << int(cmyk._k*100) << L"%, ";
	std::wstring cms = cmos.str();

	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);
	SolidBrush textBr(theme->GetColor(Theme::ColorText));
	g.DrawString(rgb.c_str(), (int)rgb.length(), theme->GetGUIFont(), Area(textLeft, textTop, rc.GetWidth()-textLeft-KWheelMargin, 24), &sf, &textBr);
	g.DrawString(cms.c_str(), (int)cms.length(), theme->GetGUIFont(), Area(textLeft, textTop+24, rc.GetWidth()-textLeft-KWheelMargin, 24), &sf, &textBr);

	// Draw border
	rc.Narrow(0,0,1,1);
	SolidBrush border(theme->GetColor(Theme::ColorActiveStart));
	Pen pn(&border, 1.0f);
	g.DrawRectangle(&pn, rc);
}

void ColorPopupWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventMove && IsKeyDown(KeyMouseLeft) && x<155) {
		Pixels cx = x-KWheelMargin;
		Pixels cy = y-KWheelMargin;

		_hue = float(_wheel.GetHueAt(cx,cy));
		_sat = float(_wheel.GetSaturationAt(cx,cy));
		Update();
	}
	else if(ev==MouseEventLDown) {
		if(x>(KWheelMargin*2+150) && y > KWheelMargin+24 && y < KWheelMargin+24+12) {
			Area rc = GetClientArea();
			Pixels width = rc.GetWidth()-3*KWheelMargin-150;
			int idx = (x-(KWheelMargin*2)-145)/(width/KFavouriteColorCount);
			if(idx>=0 && idx < KFavouriteColorCount) {
				RGBColor color = _favColors[idx];
				HSVColor col = ColorSpaces::RGBToHSV(color._r, color._g, color._b);
				_hue = float(col._h);
				_sat = float(col._s);
				_val = float(col._v);
				_brightness->SetValue(_val, false);
				Update();
			}
		}
	}
}

void ColorPopupWnd::Update() {
	_brightness->SetValue(_val,false);
	_color = ColorSpaces::HSVToRGB(_hue,_sat,_val);

	EventChanged.Fire(this, NotificationChanged());
	Repaint();
}

void ColorPopupWnd::OnSize(const Area& ns) {
	Layout();
}

void ColorPopupWnd::Layout() {
	Area rc = GetClientArea();
	rc.Narrow(150+KWheelMargin, 48, KWheelMargin, KWheelMargin);
	if(_brightness) {
		_brightness->Fill(LayoutLeft, rc);
	}
}

RGBColor ColorPopupWnd::GetColor() const {
	return _color;
}

HSVColor ColorPopupWnd::GetHSVColor() const {
	return HSVColor(_hue, _sat, _val);
}

void ColorPopupWnd::SetColor(double r, double g, double b) {
	HSVColor hsv = ColorSpaces::RGBToHSV(r,g,b);
	SetColor(hsv);
}

void ColorPopupWnd::SetColor(const RGBColor& col) {
	SetColor(col._r, col._g, col._b);
}

void ColorPopupWnd::SetColor(const HSVColor& hsv) {
	_hue = float(hsv._h);
	_sat = float(hsv._s);
	_val = float(hsv._v);
	_brightness->SetValue(_val);
	Update();
}

/* ColorChooserWnd */
ColorChooserWnd::ColorChooserWnd(RGBColor* c, RGBColor* tc): ChildWnd(L""), _colorsIcon(Icons::GetIconPath(Icons::IconColorChooser)), _color(c), _tcolor(tc) {
}

ColorChooserWnd::~ColorChooserWnd() {
}

void ColorChooserWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	Area rc = GetClientArea();
	SolidBrush bbr(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&bbr, rc);

	rc.Narrow(1,1,1,1);
	SolidBrush cbr(*_color);
	g.FillRectangle(&cbr, rc);

	Area iconArea(rc.GetRight()-20, rc.GetTop(), 16, 16);
	g.DrawImage(_colorsIcon, iconArea);
}

void ColorChooserWnd::Notify(ref<Object> source, const ColorPopupWnd::NotificationChanged& data) {
	if(source==ref<Object>(_cpw)) {
		*_color = _cpw->GetColor();
		if(_tcolor!=0) *_tcolor = *_color;
		Repaint();
	}
}

void ColorChooserWnd::OnSize(const Area& ns) {
	Repaint();
}

void ColorChooserWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLUp) {
		if(!_cpw) {
			_cpw = GC::Hold(new ColorPopupWnd());
			_cpw->EventChanged.AddListener(ref<Listener<ColorPopupWnd::NotificationChanged> >(this));
		}

		Area rc = GetClientArea();
		_cpw->PopupAt(0,rc.GetHeight()+1,this);
		_cpw->SetColor(*_color);
		ModalLoop ml;
		ml.Enter(_cpw->GetWindow(), false);
		_cpw->Show(false);
	}
	else if(ev==MouseEventRDown) {
		#ifdef _WIN32	
			COLORREF g_rgbBackground = RGB(BYTE((_color->_r)*255.0), BYTE((_color->_g)*255.0), BYTE((_color->_b)*255.0));
			COLORREF g_rgbCustom[16] = {0};

			CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};

			cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR;
			cc.hwndOwner = GetWindow();
			cc.rgbResult = g_rgbBackground;
			cc.lpCustColors = g_rgbCustom;

			if(ChooseColor(&cc)) {
				_color->_r = double(GetRValue(cc.rgbResult))/255.0;
				_color->_g = double(GetGValue(cc.rgbResult))/255.0;
				_color->_b = double(GetBValue(cc.rgbResult))/255.0;
				if(_tcolor!=0) *_tcolor = *_color;
			}
		#endif

		Repaint();
	}
}

ColorProperty::ColorProperty(const std::wstring& name, RGBColor* color, RGBColor* tc): Property(name), _color(color), _tcolor(tc)  {
}

ColorProperty::~ColorProperty() {
}

ref<Wnd> ColorProperty::GetWindow() {
	if(!_wnd) {
		_wnd = GC::Hold(new ColorChooserWnd(_color, _tcolor));
	}

	return _wnd;
}

void ColorProperty::Update() {
	// kleurtje van de knop updaten
}