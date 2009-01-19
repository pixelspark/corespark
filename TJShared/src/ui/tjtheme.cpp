#include "../../include/ui/tjui.h" 
#include "../../resource.h"
using namespace tj::shared::graphics;
using namespace tj::shared;

const float Theme::KDefaultDPI = 96.0f;

Theme::Theme(float dpi) {
	_dpi = dpi/KDefaultDPI;
	_font = 0;
	_fontBold = 0;
	_fontSmall = 0;
	_fontLink = 0;
	_grab = LoadCursor(GetModuleHandle(L"TJShared"), MAKEINTRESOURCE(IDC_GRABHAND));
	_grabbed = LoadCursor(GetModuleHandle(L"TJShared"), MAKEINTRESOURCE(IDC_GRAB));
}

Theme::~Theme() {
	delete _font;
	delete _fontBold;
	delete _fontSmall;
	delete _fontLink;
	DestroyCursor(_grab);
	DestroyCursor(_grabbed);
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
	ret.SetWidth(Pixels(bound.Width / df));
	ret.SetHeight(Pixels(bound.Height / df));
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
	LinearGradientBrush br(origin, bottom, ChangeAlpha((ColorToolbarStart), alphaInt), ChangeAlpha(GetColor(ColorToolbarEnd), alphaInt));
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

	Region oldClip;
	g.GetClip(&oldClip);
	g.SetClip(rc, CombineModeIntersect);
	DrawHighlightEllipse(g, highlight, 0.61f);
	g.SetClip(&oldClip);

	Pixels shadowSize = GetMeasureInPixels(MeasureShadowSize);
	Area shadow = rc;
	shadow.SetHeight(shadowSize);
	LinearGradientBrush shadowBrush(PointF(0.0f, (float)shadow.GetTop()-1.0f), PointF(0.0f, (float)shadow.GetBottom()), GetColor(ColorShadow), ChangeAlpha(GetColor(ColorShadow), 0));
	g.FillRectangle(&shadowBrush, shadow);

	Pen shadowLine(GetColor(ColorShadowed), 1.0f);
	g.DrawLine(&shadowLine, rc.GetLeft(), rc.GetTop(), rc.GetRight(), rc.GetTop());
}

void Theme::DrawShadowRectangle(graphics::Graphics& g, const Area& c, float alpha) {
	static REAL blendPositions[3] = {0.0f, 0.2f, 1.0f};
	static REAL blendFactors[3] = {1.0f, 0.0f, 0.0f};
	static const Pixels KFocusRectangleWidth = 6;
	static Color KSurroundColors[1] = { Color::Transparent };

	Area rc = c;
	GraphicsPath path;
	rc.Widen(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	path.AddRectangle(rc);
	PathGradientBrush gbrush(&path);
	gbrush.SetCenterColor(ChangeAlpha(GetColor(ColorShadow), int(alpha*255.0f)));
	int KNumSurroundColors = 1;
	gbrush.SetSurroundColors(KSurroundColors, &KNumSurroundColors);

	float fx = max(0.0f, float(rc.GetWidth()-5*KFocusRectangleWidth)/float(rc.GetWidth()));
	float fy = max(0.0f, float(rc.GetHeight()-5*KFocusRectangleWidth)/float(rc.GetHeight()));
	gbrush.SetFocusScales(fx,fy);
	
	rc.Narrow(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	Pen focusPen(&gbrush, 3.0f);
	g.DrawRectangle(&focusPen, rc);
}

void Theme::DrawFocusRectangle(graphics::Graphics& g, const Area& c, float alpha) {
	static REAL blendPositions[3] = {0.0f, 0.2f, 1.0f};
	static REAL blendFactors[3] = {1.0f, 0.0f, 0.0f};
	static const Pixels KFocusRectangleWidth = 6;
	static Color KSurroundColors[1] = { Color::Transparent };

	// try to escape the clipping
	GraphicsContainer gcc = g.BeginContainer();
	g.ResetClip();

	Area rc = c;
	GraphicsPath path;
	rc.Widen(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	path.AddRectangle(rc);
	PathGradientBrush gbrush(&path);
	gbrush.SetCenterColor(ChangeAlpha(GetColor(ColorFocus), int(alpha*255.0f)));
	int KNumSurroundColors = 1;
	gbrush.SetSurroundColors(KSurroundColors, &KNumSurroundColors);

	float fx = max(0.0f, float(rc.GetWidth()-5*KFocusRectangleWidth)/float(rc.GetWidth()));
	float fy = max(0.0f, float(rc.GetHeight()-5*KFocusRectangleWidth)/float(rc.GetHeight()));
	gbrush.SetFocusScales(fx,fy);
	
	rc.Narrow(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	Pen focusPen(&gbrush, 3.0f);
	g.DrawRectangle(&focusPen, rc);
	g.EndContainer(gcc);
}

void Theme::DrawFocusEllipse(graphics::Graphics& g, const Area& c, float alpha) {
	static REAL blendPositions[3] = {0.0f, 0.5f, 1.0f};
	static REAL blendFactors[3] = {1.0f, 0.3f, 0.0f};
	static const Pixels KFocusRectangleWidth = 4;
	static Color KSurroundColors[1] = { Color::Transparent };

	// try to escape the clipping
	GraphicsContainer gcc = g.BeginContainer();
	g.ResetClip();
	g.SetSmoothingMode(SmoothingModeHighQuality);

	Area rc = c;
	GraphicsPath path;
	rc.Widen(KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth,KFocusRectangleWidth);
	path.AddEllipse(rc);
	PathGradientBrush gbrush(&path);
	gbrush.SetCenterColor(ChangeAlpha(GetColor(ColorFocus), int(alpha*255.0f)));
	int KNumSurroundColors = 1;
	gbrush.SetSurroundColors(KSurroundColors, &KNumSurroundColors);

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
	static REAL blendPositions[2] = {0.0f, 1.0f};
	static REAL blendFactors[2] = {1.0f, 0.0f};
	static Color KSurroundColors[1] = { Color::Transparent };

	Area rc = c;
	GraphicsPath path;
	path.AddEllipse(rc);
	PathGradientBrush gbrush(&path);
	gbrush.SetCenterColor(Color(int(alpha*255.0f), 255, 255, 255));
	int KNumSurroundColors = 1;
	gbrush.SetSurroundColors(KSurroundColors, &KNumSurroundColors);
	
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

HCURSOR Theme::GetGrabCursor() const {
	return _grab;
}

HCURSOR Theme::GetGrabbedCursor() const {
	return _grabbed;
}

graphics::Color Theme::ChangeAlpha(const graphics::Color& col, float a) {
	return graphics::Color(BYTE(a*255.0f), col.GetR(), col.GetG(), col.GetB());
}

graphics::Color Theme::ChangeAlpha(const graphics::Color& col, int a) {
	return graphics::Color(a, col.GetR(), col.GetG(), col.GetB());
}

graphics::Font* Theme::GetGUIFontBold() const {
	if(_fontBold==0) {
		_fontBold = new Font(L"Tahoma", 11, FontStyleBold, UnitPixel, 0);
	}
	return _fontBold;
}

graphics::Font* Theme::GetGUIFont() const {
	if(_font==0) {
		_font = new Font(L"Tahoma", 11, FontStyleRegular, UnitPixel, 0);
	}
	return _font;
}


graphics::Font* Theme::GetLinkFont() const {
	if(_fontLink==0) {
		_fontLink = new Font(L"Tahoma", 11, FontStyleUnderline, UnitPixel, 0);
	}
	return _fontLink;
}

graphics::Font* Theme::GetGUIFontSmall() const {
	if(_fontSmall==0) {
		_fontSmall = new Font(L"Tahoma", 9, FontStyleRegular, UnitPixel, 0);
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
			return Color::LightBlue;

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
			return Color(201,201,201);

		case ColorProgressBackgroundEnd:
			return Color(255,255,255);

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

	graphics::LinearGradientBrush* lbr = new graphics::LinearGradientBrush(PointF(0.0f, -float(childrc.top-rootrc.top)), PointF(0.0f,float(rootrc.bottom-rootrc.top)), Color(0,0,0), Color(90,90,90));
	lbr->SetWrapMode(WrapModeClamp);
	REAL factors[3] = {1.0f, 0.0f, 0.0f};
	REAL positions[3] = {0.0f, 0.25f ,1.0f};
	lbr->SetBlend(factors,positions, 3);

	return lbr;
}