#include "../../include/ui/tjui.h" 
using namespace tj::shared::graphics;
using namespace tj::shared;

const float Theme::KDefaultDPI = 96.0f;

Theme::Theme(float dpi) {
	_dpi = dpi/KDefaultDPI;
	_font = 0;
	_fontBold = 0;
	_fontSmall = 0;
	_fontLink = 0;
}

Theme::~Theme() {
	delete _font;
	delete _fontBold;
	delete _fontSmall;
	delete _fontLink;
}

const wchar_t* Icons::_paths[Icons::_IconLast] = {
	L"icons/shared/check.png",
	L"icons/shared/colors.png",
	L"icons/shared/file.png",
	L"icons/browser/back.png",
	L"icons/browser/forward.png",
	L"icons/browser/stop.png",
	L"icons/browser/reload.png",
	L"icons/browser/go.png",
	L"icons/shared/radiocheck.png",
	L"icons/shared/ok.png",
	L"icons/shared/path-separator.png",
	L"icons/shared/expand.png",
	L"icons/shared/collapse.png",
	L"icons/shared/tab-add.png",
	L"icons/shared/tab-close.png",
	L"icons/shared/tip.png",
	L"icons/shared/search.png",
	L"icons/shared/down-arrow.png",
	L"icons/shared/grabber.png",
	L"icons/shared/errors/message.png",
	L"icons/shared/errors/notify.png",
	L"icons/shared/errors/error.png",
	L"icons/shared/errors/fatal.png",
	L"icons/shared/copy.png",
	L"icons/shared/cut.png", 
	L"icons/shared/paste.png",
	L"icons/shared/submenu.png",
	L"icons/shared/tab-add-active.png",
	L"icons/shared/tab-close-active.png",
	L"icons/shared/keyboard.png",
};

std::wstring Icons::GetIconPath(IconIdentifier i) {
	if(i<_IconLast) {
		return _paths[(unsigned int)i];
	}

	Throw(L"Icon with specified does not exist (programming error)!", ExceptionTypeError);
}

Area Theme::MeasureText(const std::wstring& text, graphics::Font* font) const {
	HDC dc = GetDC(NULL);
	float df = GetDPIScaleFactor();
	Area ret;
	RectF bound;
	{
		Graphics g(dc);
		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);
		g.MeasureString(text.c_str(), (int)text.length(), font, PointF(0.0f, 0.0f), &sf, &bound);
	}
	ReleaseDC(NULL, dc);
	ret.SetWidth(Pixels(bound.GetWidth() / df));
	ret.SetHeight(Pixels(bound.GetHeight() / df));
	return ret;
}

/** DPI Stuff */
float Theme::GetDPIScaleFactor() const {
	return _dpi;
}

void Theme::DrawToolbarBackground(graphics::Graphics& g, float x, float y, float w, float h, float alpha) {
	DrawToolbarBackground(g,Area(Pixels(x),Pixels(y),Pixels(w),Pixels(h)), alpha);
}

void Theme::DrawToolbarBackground(graphics::Graphics& g, float x, float y, float w, float h) {
	DrawToolbarBackground(g,Area(Pixels(x),Pixels(y),Pixels(w),Pixels(h)));
}

void Theme::DrawToolbarBackground(graphics::Graphics& g, const Area& rc) {
	SolidBrush zwart(GetColor(ColorBackground));
	g.FillRectangle(&zwart, rc);

	PointF origin(float(rc.GetLeft()), float(rc.GetTop()));
	PointF bottom(float(rc.GetLeft()), float(rc.GetBottom()));
	LinearGradientBrush br(origin, bottom, GetColor(ColorToolbarStart), GetColor(ColorToolbarEnd));
	SolidBrush dbr(GetColor(ColorDisabledOverlay));
	g.FillRectangle(&br, rc);
	g.FillRectangle(&dbr, rc);

	LinearGradientBrush glas(origin, PointF(float(rc.GetLeft()),rc.GetTop()+float(rc.GetHeight())/2.0f), GetColor(ColorGlassStart), GetColor(ColorGlassEnd));
	g.FillRectangle(&glas, RectF(float(rc.GetLeft()), float(rc.GetTop()), float(rc.GetWidth()), float(rc.GetHeight())/2.0f));
}

void Theme::DrawToolbarBackground(graphics::Graphics& g, const Area& rc, float alpha) {
	int alphaInt = min(255,int(alpha*255.0f));
	int alphaBack = min(255,int(alpha*0.25f*255.0f));
	SolidBrush zwart(ChangeAlpha(GetColor(ColorBackground), alphaBack));
	g.FillRectangle(&zwart, rc);

	PointF origin(float(rc.GetLeft()), float(rc.GetTop()));
	PointF bottom(float(rc.GetLeft()), float(rc.GetBottom()));
	LinearGradientBrush br(origin, bottom, ChangeAlpha(GetColor(ColorToolbarStart), alphaInt), ChangeAlpha(GetColor(ColorToolbarEnd), alphaInt));
	SolidBrush dbr(GetColor(ColorDisabledOverlay));
	g.FillRectangle(&br, rc);
	g.FillRectangle(&dbr, rc);

	LinearGradientBrush glas(origin, PointF(float(rc.GetLeft()),rc.GetTop()+float(rc.GetHeight())/2.0f), GetColor(ColorGlassStart), GetColor(ColorGlassEnd));
	g.FillRectangle(&glas, RectF(float(rc.GetLeft()), float(rc.GetTop()), float(rc.GetWidth()), float(rc.GetHeight())/2.0f));
}

void Theme::DrawInsetRectangleLight(graphics::Graphics& g, const Area& rc) {
	const static Pixels KShadowSize = 5;

	LinearGradientBrush shadowGradientBrush(PointF((float)rc.GetLeft(),(float)rc.GetTop()), PointF(float(rc.GetLeft()+KShadowSize+1), (float)rc.GetTop()), ChangeAlpha(GetColor(ColorShadow),32), ChangeAlpha(GetColor(ColorShadow), 0));
	g.FillRectangle(&shadowGradientBrush, Area(rc.GetLeft()-2, rc.GetTop(), KShadowSize, rc.GetHeight()));

	LinearGradientBrush shadowGradientBrushHorizontal(PointF((float)rc.GetLeft(),(float)rc.GetTop()), PointF(float(rc.GetLeft()), float(rc.GetTop()+KShadowSize+1)), ChangeAlpha(GetColor(ColorShadow),32), ChangeAlpha(GetColor(ColorShadow), 0));
	g.FillRectangle(&shadowGradientBrushHorizontal, Area(rc.GetLeft(), rc.GetTop(), rc.GetWidth(), KShadowSize));
}

void Theme::DrawInsetRectangle(graphics::Graphics& g, const Area& rc) {
	SolidBrush shadowed(GetColor(ColorShadowed));
	g.FillRectangle(&shadowed, rc);

	Area highlight = rc;
	Pixels halfWidth = highlight.GetWidth()/4;
	highlight.Widen(halfWidth,highlight.GetHeight()*2,halfWidth,0);

	g.SetClip(SimpleRectangle<float>(rc));
	DrawHighlightEllipse(g, highlight, 0.61f);
	g.ResetClip();

	Pixels shadowSize = GetMeasureInPixels(MeasureShadowSize);
	Area shadow = rc;
	shadow.SetHeight(shadowSize);
	LinearGradientBrush shadowBrush(PointF(0.0f, (float)shadow.GetTop()-1.0f), PointF(0.0f, (float)shadow.GetBottom()), GetColor(ColorShadow), ChangeAlpha(GetColor(ColorShadow), 0));
	g.FillRectangle(&shadowBrush, shadow);

	Pen shadowLine(GetColor(ColorShadowed), 1.0f);
	g.DrawLine(&shadowLine, rc.GetLeft(), rc.GetTop(), rc.GetRight(), rc.GetTop());
}

void Theme::DrawShadowRectangle(graphics::Graphics& g, const Area& c, float alpha) {
	static float blendPositions[3] = {0.0f, 0.2f, 1.0f};
	static float blendFactors[3] = {1.0f, 0.0f, 0.0f};
	static const Pixels KFocusRectangleWidth = 6;
	static Color KSurroundColors[1] = { Color() };

	Area rc = c;
	GraphicsPath path;
	rc.Widen(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	path.AddRectangle(rc);
	PathGradientBrush gbrush(&path);
	gbrush.SetCenterColor(ChangeAlpha(GetColor(ColorShadow), int(alpha*255.0f)));
	gbrush.SetSurroundColors(KSurroundColors, 1);

	float fx = max(0.0f, float(rc.GetWidth()-5*KFocusRectangleWidth)/float(rc.GetWidth()));
	float fy = max(0.0f, float(rc.GetHeight()-5*KFocusRectangleWidth)/float(rc.GetHeight()));
	gbrush.SetFocusScales(fx,fy);
	
	rc.Narrow(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	Pen focusPen(&gbrush, 3.0f);
	g.DrawRectangle(&focusPen, rc);
}

void Theme::DrawFocusRectangle(graphics::Graphics& g, const Area& c, float alpha) {
	static float blendPositions[3] = {0.0f, 0.05f, 1.0f};
	static float blendFactors[3] = {1.0f, 0.0f, 0.0f};
	static const Pixels KFocusRectangleWidth = 6;
	static Color KSurroundColors[1] = { ChangeAlpha(GetColor(ColorFocus), 0) };

	// try to escape the clipping
	GraphicsContainer gcc = g.BeginContainer();
	g.ResetClip();

	Area rc = c;
	GraphicsPath path;
	rc.Widen(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	path.AddRectangle(rc);
	PathGradientBrush gbrush(&path);
	gbrush.SetCenterColor(ChangeAlpha(GetColor(ColorFocus), int(alpha*255.0f)));
	gbrush.SetSurroundColors(KSurroundColors, 1);

	float fx = max(0.0f, float(rc.GetWidth()-5*KFocusRectangleWidth)/float(rc.GetWidth()));
	float fy = max(0.0f, float(rc.GetHeight()-5*KFocusRectangleWidth)/float(rc.GetHeight()));
	gbrush.SetFocusScales(fx,fy);
	
	rc.Narrow(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	Pen focusPen(&gbrush, float(KFocusRectangleWidth/2));
	//g.DrawRectangle(&focusPen, rc);
	if(rc.GetWidth()>5 && rc.GetHeight()>5) {
		g.DrawRoundRectangle(&focusPen, rc, 5.0f);
	}
	else {
		g.DrawRect(&focusPen, rc);
	}
	g.EndContainer(gcc);
}

void Theme::DrawFocusEllipse(graphics::Graphics& g, const Area& c, float alpha) {
	static float blendPositions[3] = {0.0f, 0.5f, 1.0f};
	static float blendFactors[3] = {1.0f, 0.3f, 0.0f};
	static const Pixels KFocusRectangleWidth = 4;
	static Color KSurroundColors[1] = { Color() };

	// try to escape the clipping
	GraphicsContainer gcc = g.BeginContainer();
	g.ResetClip();

	Area rc = c;
	GraphicsPath path;
	rc.Widen(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	path.AddEllipse(rc);
	PathGradientBrush gbrush(&path);
	gbrush.SetCenterColor(ChangeAlpha(GetColor(ColorFocus), int(alpha*255.0f)));
	gbrush.SetSurroundColors(KSurroundColors, 1);

	float fx = max(0.0f, float(rc.GetWidth()-5*KFocusRectangleWidth)/float(rc.GetWidth()));
	float fy = max(0.0f, float(rc.GetHeight()-5*KFocusRectangleWidth)/float(rc.GetHeight()));
	gbrush.SetFocusScales(fx,fy);	
	g.FillRectangle(&gbrush, rc);

	// Draw white border
	SolidBrush borderBrush(GetColor(ColorBackground));
	Pen border(&borderBrush, 1.0f);
	g.DrawEllipse(&border, c);
	g.EndContainer(gcc);
}

void Theme::DrawMessageBar(graphics::Graphics& g, const Area& header) {
	SolidBrush back(GetColor(Theme::ColorBackground));
	g.FillRectangle(&back, header);

	LinearGradientBrush headerBrush(PointF(0.0f, 0.0f), PointF(0.0f, float(header.GetHeight())), GetColor(Theme::ColorActiveStart), GetColor(Theme::ColorActiveEnd));
	g.FillRectangle(&headerBrush, header);

	Area ellipse = header;
	ellipse.Widen(0,0,0,ellipse.GetHeight());
	DrawHighlightEllipse(g, ellipse, 0.5f);

	Area glass = header;
	glass.Narrow(0,0,0,2);
	LinearGradientBrush glassHeaderBrush(PointF(0.0f, 0.0f), PointF(0.0f, float(header.GetHeight())), GetColor(Theme::ColorGlassStart), GetColor(Theme::ColorGlassEnd));
	g.FillRectangle(&glassHeaderBrush, glass);
}

void Theme::DrawHighlightEllipse(graphics::Graphics& g, const Area& c, float alpha) {
	static float blendPositions[2] = {0.0f, 1.0f};
	static float blendFactors[2] = {1.0f, 0.0f};
	static Color KSurroundColors[1] = { Color() };

	Area rc = c;
	GraphicsPath path;
	path.AddEllipse(rc);
	PathGradientBrush gbrush(&path);
	gbrush.SetCenterColor(Color(int(alpha*255.0f), 255, 255, 255));
	gbrush.SetSurroundColors(KSurroundColors, 1);
	
	g.FillEllipse(&gbrush, rc);
}

Pixels Theme::GetMeasureInPixels(Measure m) const {
	switch(m) {
		case MeasureDialogHeaderHeight:
			return 30;

		case MeasureToolbarHeight:
		case MeasureListHeaderHeight:
			return 24;

		case MeasureListItemHeight:
		case MeasureMenuItemHeight:
			return 19;

		case MeasurePropertyHeight:
			return 17;

		case MeasureShadowSize:
			return 5;

		case MeasureSliderDraggerWidth:
			return 16;

		case MeasureMaximumSnappingDistance:
			return 2;

		case MeasureMinimumContextMenuWidth:
			return 150;
	}

	return -1;
}

graphics::Color Theme::ChangeAlpha(const graphics::Color& col, float a) {
	return graphics::Color(BYTE(a*255.0f), col.GetR(), col.GetG(), col.GetB());
}

graphics::Color Theme::ChangeAlpha(const graphics::Color& col, int a) {
	return graphics::Color((unsigned char)a, col.GetR(), col.GetG(), col.GetB());
}

std::wstring Theme::GetGUIFontName() const {
	return L"Tahoma";
}

graphics::Font* Theme::GetGUIFontBold() const {
	if(_fontBold==0) {
		_fontBold = new Font(GetGUIFontName().c_str(), 11, FontStyleBold);
	}
	return _fontBold;
}

graphics::Font* Theme::GetGUIFont() const {
	if(_font==0) {
		_font = new Font(GetGUIFontName().c_str(), 11, FontStyleRegular);
	}
	return _font;
}


graphics::Font* Theme::GetLinkFont() const {
	if(_fontLink==0) {
		_fontLink = new Font(GetGUIFontName().c_str(), 11, FontStyleUnderline);
	}
	return _fontLink;
}

graphics::Font* Theme::GetGUIFontSmall() const {
	if(_fontSmall==0) {
		_fontSmall = new Font(GetGUIFontName().c_str(), 9, FontStyleRegular);
	}
	return _fontSmall;
}


std::wstring Theme::GetName() const { 
	if(_dpi!=1.0f) {
		return TL(theme_default) + std::wstring(L" (") + Stringify(_dpi*KDefaultDPI) + L" dpi) ";
	}
	else {
		return TL(theme_default);
	}
}

graphics::Color Theme::GetColor(const ColorIdentifier& ci) const {
	switch(ci) {
		case ColorBackground:
		case ColorTabButtonBackground:
		case ColorEditBackground:
		case ColorPropertyBackground:
		case ColorVideoBackground:
			return Color(0,0,0);

		case ColorTimeBackground:
			return Color(15,15,15);

		case ColorLine:
			return Color(50,50,50);

		case ColorLink:
			return Color(0,0,255);

		case ColorFocus:
			return Color(0xAD, 0xD8, 0xE6);

		case ColorShadow:
			return Color(200,0,0,0);

		case ColorShadowed:
			return Color(255/3, 255, 255, 255);

		case ColorText:
			return Color(255,255,255);

		case ColorActiveTrack:
			return Color(25,25,25);

		case ColorActiveStart:
		case ColorHint:
		case ColorToolbarStart:
			return Color(230,130,130,130);

		case ColorActiveEnd:
		case ColorToolbarEnd:
			return Color(230,90,90,90);

		case ColorHighlightStart:
			return Color(255,127,75,0);

		case ColorHighlightEnd:
			return Color(255,255,150,0);

		case ColorCommandMarker:
			return Color(255,0,0);

		case ColorSplitterStart:
			return Color(0,0,0);

		case ColorSplitterEnd:
			return Color(0,0,0);

		case ColorGlassStart:
			return Color(0,255,255,255);

		case ColorGlassEnd:
			return Color(15,255,255,255);

		case ColorCurrentPosition:
			return Color(255,255,255);

		case ColorDescriptionText:
			return Color(120,120,120);

		case ColorTimeSelectionEnd:
			return Color(100,100,50, 0);

		case ColorTimeSelectionStart:
			return Color(100,200,100, 0);

		case ColorDisabledOverlay:
			return Color(200,0,0,0);

		case ColorFader:
			return Color(255,255,255);

		case ColorTabButtonStart:
			return Color(100,110,110,110);

		case ColorTabButtonEnd:
			return Color(100,70,70,70);

		case ColorProgressBackgroundStart:
			return Color(0,0,0);

		case ColorProgressBackgroundEnd:
			return Color(54,54,54);

		case ColorProgressGlassStart:
			return Color(211,255,255,255);

		case ColorProgressGlassEnd:
			return Color(43,255,255,255);

		case ColorProgress:
			return Color(0,211,40);

		case ColorNone:
		default:
			return Color(0,0,0,0);
	}
}

Color Theme::GetSliderColorStart(const SliderType& i) const {
	switch(i) {
		case SliderMaster:
			return Color(255, 200,25,25);

		case SliderSubmix:
			return Color(255, 25, 200, 25);

		case SliderMacro:
			return Color(255, 60, 60, 110);

		case SliderAlpha:
			return Color(200, 200, 60);

		default:
			return GetColor(ColorActiveStart);
	}
}

Color Theme::GetSliderColorEnd(const SliderType& i) const {
	switch(i) {
		case SliderMaster:
			return Color(150, 200,25,25);

		case SliderSubmix:
			return Color(150, 25, 200, 25);

		case SliderMacro:
			return Color(150, 60, 60, 110);

		case SliderAlpha:
			return Color(150, 150, 60);

		default:
			return GetColor(ColorActiveEnd);
	}
}

Brush* Theme::GetApplicationBackgroundBrush(HWND root, HWND child) const {
	RECT rootrc, childrc;
	GetWindowRect(root, &rootrc);
	GetWindowRect(child, &childrc);

	graphics::LinearGradientBrush* lbr = new graphics::LinearGradientBrush(PointF(0.0f, -float(childrc.top-rootrc.top)), PointF(0.0f,-float(childrc.top-rootrc.bottom)), Color(90,90,90), Color(0,0,0));
	lbr->SetBlendPosition(0.1f);
	return lbr;
}

/** TokenizedPainter **/
const Pixels TokenizedTextPainter::KDefaultMargin = 5;

TokenizedTextPainter::TokenizedTextPainter(const Area& rc, strong<Theme> theme): _rc(rc), _theme(theme), _margin(KDefaultMargin) {
}

TokenizedTextPainter::~TokenizedTextPainter() {
}

void TokenizedTextPainter::DrawToken(graphics::Graphics& g, const std::wstring& txt, graphics::Font* font, graphics::Brush* textBrush, graphics::Brush* backgroundBrush, graphics::Pen* borderPen, graphics::Brush* shadowBrush) {
	RectF bound;
	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);

	g.MeasureString(txt.c_str(), (int)txt.length(), font, _rc, &sf, &bound);

	if(borderPen!=0) {
		RectF borderBound = bound;
		if(shadowBrush!=0) {
			borderBound.SetX(borderBound.GetX()+0.5f);
			borderBound.SetY(borderBound.GetY()+0.5f);
		}

		g.DrawRectangle(borderPen, borderBound);
	}

	if(backgroundBrush!=0) {
		g.FillRectangle(backgroundBrush, bound);
	}

	if(shadowBrush!=0) {
		AreaF shadowRC = _rc;
		shadowRC.Translate(0.8f, 0.8f);
		g.DrawString(txt.c_str(), (size_t)txt.length(), font, shadowRC, &sf, shadowBrush);
	}

	g.DrawString(txt.c_str(), (int)txt.length(), font, _rc, &sf, textBrush);
	_rc.Narrow(Pixels(bound.GetWidth()) + _margin,0,0,0);
}

void TokenizedTextPainter::DrawToken(graphics::Graphics& g, const std::wstring& txt, bool bold, Brush* textBrush, Brush* backgroundBrush, Pen* borderPen, bool shadow) {
	Font* font = bold ? _theme->GetGUIFontBold() : _theme->GetGUIFont();

	Brush* shadowBrush = 0;
	SolidBrush shadowBrushReal(Theme::ChangeAlpha(_theme->GetColor(Theme::ColorBackground),172));
	if(shadow) {
		shadowBrush = &shadowBrushReal;
	}
	DrawToken(g, txt, font, textBrush, backgroundBrush, borderPen, shadowBrush);
}

Pixels TokenizedTextPainter::GetMargin() const {
	return _margin;
}

void TokenizedTextPainter::SetMargin(const Pixels& m) {
	_margin = m;
}