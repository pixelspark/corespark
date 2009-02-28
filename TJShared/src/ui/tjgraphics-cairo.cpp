#ifdef TJSHARED_USE_CAIRO
#include "../../include/tjshared.h"
#include "../../include/ui/tjui.h"
#include "../../include/ui/tjgraphics.h"
#include <cairo/cairo.h>
using namespace tj::shared;
using namespace tj::shared::graphics;

#ifdef WIN32
	#include <cairo/cairo-win32.h>

	char* ToUTF8(const wchar_t* text) {
		int r = WideCharToMultiByte(CP_UTF8, 0, text, wcslen(text), NULL, 0, NULL, NULL);
		char* buffer = new char[r+1];
		buffer[r] = 0;
		WideCharToMultiByte(CP_UTF8, 0, text, wcslen(text), buffer, r, NULL, NULL);
		return buffer;
	}
#endif

struct PenPrivate {
	cairo_pattern_t* pattern;
	float width;
	Color color;
};

struct FontPrivate {
	std::wstring family;
	float size;
	FontStyle style;
};

Brush::~Brush() {
	cairo_pattern_t* cp = reinterpret_cast<cairo_pattern_t*>(_private);
	if(cp!=0) {
		cairo_pattern_destroy(cp);
	}
}

/** Pen **/
Pen::Pen(Brush* brush, float width) {
	PenPrivate* pp = new PenPrivate;
	_private = reinterpret_cast<void*>(pp);

	cairo_pattern_t* pattern = reinterpret_cast<cairo_pattern_t*>(brush->_private);
	cairo_pattern_reference(pattern);
	pp->pattern = pattern;
	pp->width = width;
}

Pen::Pen(const Color& color, float width) {
	PenPrivate* pp = new PenPrivate;
	_private = reinterpret_cast<void*>(pp);
	pp->pattern = cairo_pattern_create_rgba(color._r, color._g, color._b, color._a);
	pp->width = width;
	pp->color = color;
}

Pen::~Pen() {
	PenPrivate* pp = reinterpret_cast<PenPrivate*>(_private);
	if(pp!=0) {
		if(pp->pattern != 0) {
			cairo_pattern_destroy(pp->pattern);
		}
	}
	delete pp;
}

Color Pen::GetColor() const {
	PenPrivate* pp = reinterpret_cast<PenPrivate*>(_private);
	return pp->color;
}

/** SolidBrush **/
SolidBrush::SolidBrush(const Color& col) {
	_private = reinterpret_cast<void*>(cairo_pattern_create_rgba(col._r, col._g, col._b, col._a));
}

SolidBrush::~SolidBrush() {
	// cairo_pattern_t* is deleted in ~Brush
}

/** Font **/
Font::Font(const wchar_t* family, float pixelSize, FontStyle style) {
	FontPrivate* fp = new FontPrivate;
	fp->family = family;
	fp->size = pixelSize;
	fp->style = style;
	_private = reinterpret_cast<void*>(fp);

}
Font::~Font() {
	delete reinterpret_cast<FontPrivate*>(_private);
}

/** LinearGradientBrush **/
LinearGradientBrush::LinearGradientBrush(const PointF& a, const PointF& b, const Color& start, const Color& end) {
	cairo_pattern_t* pattern = cairo_pattern_create_linear(a._x, a._y, b._x, b._y);
	cairo_pattern_add_color_stop_rgba(pattern, 0.0, start._r, start._g, start._b, start._a);
	cairo_pattern_add_color_stop_rgba(pattern, 1.0, end._r, end._g, end._b, end._a);
	_private = reinterpret_cast<void*>(pattern);
}

LinearGradientBrush::~LinearGradientBrush() {
	// _private deleted in ~Brush
}

void LinearGradientBrush::SetBlendPosition(float b) {
	//XX
}

/** PathGradientBrush **/
PathGradientBrush::PathGradientBrush(PointF* points, unsigned int pointCount) {
	_private = 0;
	//XX
}

PathGradientBrush::PathGradientBrush(GraphicsPath* gp) {
	_private = 0;
	//XX
}

PathGradientBrush::~PathGradientBrush() {
	// _private is deleted in ~Brush
}

void PathGradientBrush::SetFocusScales(float fx, float fy) {
	//XX
}

void PathGradientBrush::SetCenterColor(const Color& col) {
	//XX
}

void PathGradientBrush::SetCenterPoint(const PointF& point) {
	//XX
}

void PathGradientBrush::SetSurroundColors(Color* colors, unsigned int colorCount) {
	//XX
}

/** Image **/
Image* Image::FromFile(const wchar_t* path, bool useICM) {
	Image* image = new Image;
	char* utfPath = ToUTF8(path);
	cairo_surface_t* cs = cairo_image_surface_create_from_png(utfPath);
	delete[] utfPath;
	
	switch(cairo_surface_status(cs)) {
		case CAIRO_STATUS_NO_MEMORY:
			Log::Write(L"TJShared/Cairo", L"CAIRO_STATUS_NO_MEMORY");
			break;

		case CAIRO_STATUS_FILE_NOT_FOUND:
			Log::Write(L"TJShared/Cairo", L"CAIRO_STATUS_FILE_NOT_FOUND "+std::wstring(path));
			break;
	}
	
	image->_private = reinterpret_cast<void*>(cs);
	return image;
}

Image::~Image() {
	cairo_surface_t* cs = reinterpret_cast<cairo_surface_t*>(_private);
	if(cs!=0) {
		cairo_surface_destroy(cs);
	}
}

int Image::GetWidth() {
	cairo_surface_t* cs = reinterpret_cast<cairo_surface_t*>(_private);
	return cairo_image_surface_get_width(cs);
}

int Image::GetHeight() {
	cairo_surface_t* cs = reinterpret_cast<cairo_surface_t*>(_private);
	return cairo_image_surface_get_height(cs);
}

bool Image::Save(const wchar_t* path, const wchar_t* mime) {
	return false; //XX
}

/** BitmapData **/
BitmapData::BitmapData() {
	_private = 0;
}

BitmapData::~BitmapData() {
	if(_private!=0) {
		cairo_surface_destroy(reinterpret_cast<cairo_surface_t*>(_private));
	}
}

unsigned char* BitmapData::GetPointer() const {
	cairo_surface_t* surface = reinterpret_cast<cairo_surface_t*>(_private);
	return cairo_image_surface_get_data(surface);
}

unsigned int BitmapData::GetStride() const {
	cairo_surface_t* surface = reinterpret_cast<cairo_surface_t*>(_private);
	return cairo_image_surface_get_stride(surface);
}

/** Bitmap **/
Bitmap::Bitmap(unsigned int w, unsigned int h) {
	cairo_surface_t* cb = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	_private = reinterpret_cast<void*>(cb);
}

Bitmap::Bitmap(unsigned int w, unsigned int h, Graphics* org) {
	cairo_surface_t* cb = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	_private = reinterpret_cast<void*>(cb);
}

Bitmap::Bitmap(unsigned int w, unsigned int h, unsigned int pitch, unsigned char* data) {
	cairo_surface_t* org = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, w, h, pitch);
	cairo_surface_t* cb = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cairo_t* dest = cairo_create(cb);
	cairo_set_source_surface(dest, org, 0.0, 0.0);
	cairo_paint(dest);
	cairo_destroy(dest);
	cairo_surface_destroy(org);
	_private = reinterpret_cast<void*>(cb);
}

Bitmap::~Bitmap() {
	// _private is deleted in Image::~Image
}

bool Bitmap::LockBits(const Rect& rc, bool write, BitmapData* data) {
	cairo_surface_t* surface = reinterpret_cast<cairo_surface_t*>(_private);
	unsigned char* pd = cairo_image_surface_get_data(surface);
	if(pd!=0) {
		cairo_surface_reference(surface); // BitmapData::~BitmapData destroys
		data->_private = reinterpret_cast<void*>(surface);
		return true;
	}
	return false;
}

void Bitmap::UnlockBits(BitmapData* data) {
}

/** GraphicsPath **/
GraphicsPath::GraphicsPath() {
	//XX
}

GraphicsPath::~GraphicsPath() {
	//XX
}

void GraphicsPath::AddRectangle(const RectF& rc) {
	//XX
}

void GraphicsPath::AddEllipse(const RectF& rc) {
	//XX
}

void GraphicsPath::AddPolygon(const PointF* points, unsigned int pointCount) {
	//XX
}

/** StringFormat **/
StringFormat::StringFormat() {
	//XX
}

StringFormat::~StringFormat() {
	//XX
}

void StringFormat::SetAlignment(StringAlignment sa) {
	//XX
}

void StringFormat::SetLineAlignment(StringAlignment sa) {
	//XX
}

void StringFormat::SetTrimming(StringTrimming st) {
	//XX
}

void StringFormat::SetFormatFlags(StringFormatFlags sf) {
	//XX
}

StringFormatFlags StringFormat::GetFormatFlags() const {
	//XX
	return 0;
}

void StringFormat::SetTabStops(float firstTabOffset, unsigned int count, const float* tabStops) {
	//XX
}

/** ImageAttributes **/
ImageAttributes::ImageAttributes() {
	_private = 0; //XX
}

ImageAttributes::~ImageAttributes() {
	//XX
}

void ImageAttributes::SetColorMatrix(const ColorMatrix* cm) {
	//XX
}

/** GraphicsInit **/
GraphicsInit::GraphicsInit() {
}

GraphicsInit::~GraphicsInit() {
}

/** Graphics **/
Graphics::Graphics(Image* bmp) {
	cairo_surface_t* target = reinterpret_cast<cairo_surface_t*>(bmp->_private);
	_private = reinterpret_cast<void*>(cairo_create(target));
}

#ifdef WIN32
	Graphics::Graphics(HWND wnd) {
		/*HDC dc = GetDC(wnd);
		cairo_surface_t* target = cairo_win32_surface_create(dc);
		_private = reinterpret_cast<void*>(cairo_create(target));	
		cairo_surface_destroy(target);
		ReleaseDC(wnd, dc);*/
		_private = 0;
	}

	Graphics::Graphics(HDC dc) {
		cairo_surface_t* target = cairo_win32_surface_create(dc);
		cairo_t* cg = cairo_create(target);
		_private = reinterpret_cast<void*>(cg);	
		cairo_surface_destroy(target);
	}
#endif

Graphics::~Graphics() {
	if(_private!=0) {
		cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
		cairo_surface_t* cs = cairo_get_target(cg);
		if(cs!=0) {
			cairo_surface_flush(cs);
		}
		cairo_destroy(cg);
	}
}

void Graphics::FillRectangle(Brush* brush, const RectF& rc) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	cairo_pattern_t* cp = reinterpret_cast<cairo_pattern_t*>(brush->_private);
	cairo_set_source(cg, cp);
	cairo_rectangle(cg, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight());
	cairo_fill(cg);
	cairo_restore(cg);
}

void Graphics::FillRectangle(Brush* brush, const Rect& rc) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	if(cg!=0) {
		cairo_pattern_t* cp = reinterpret_cast<cairo_pattern_t*>(brush->_private);
		if(cp!=0) {
			cairo_set_source(cg, cp);
			cairo_rectangle(cg, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight());
			cairo_fill(cg);
		}
	}
	cairo_restore(cg);
}

void Graphics::DrawRectangle(Pen* pen, const RectF& rc) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	PenPrivate* pp = reinterpret_cast<PenPrivate*>(pen->_private);
	cairo_pattern_t* cp = pp->pattern;
	cairo_set_source(cg, cp);
	cairo_set_line_width(cg, pp->width);
	cairo_rectangle(cg, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight());
	cairo_stroke(cg);
	cairo_restore(cg);
}

void Graphics::DrawRectangle(Pen* pen, const Rect& rc) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	PenPrivate* pp = reinterpret_cast<PenPrivate*>(pen->_private);
	cairo_pattern_t* cp = pp->pattern;
	cairo_set_source(cg, cp);
	cairo_set_line_width(cg, pp->width);
	cairo_rectangle(cg, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight());
	cairo_stroke(cg);
	cairo_restore(cg);
}

void Graphics::FillEllipse(Brush* brush, const RectF& rc) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	cairo_pattern_t* cp = reinterpret_cast<cairo_pattern_t*>(brush->_private);
	cairo_set_source(cg, cp);
	cairo_save(cg);
	cairo_translate(cg, rc.GetLeft() + rc.GetWidth() / 2.0, rc.GetTop() + rc.GetHeight() / 2.0);
	cairo_scale(cg, 1.0 / (rc.GetHeight() / 2.), 1.0 / (rc.GetWidth() / 2.0));
	cairo_arc(cg, 0.0, 0.0, 1.0, 0.0, 2 * 3.14159);
	cairo_restore(cg);
	cairo_fill(cg);
	cairo_restore(cg);
}

void Graphics::FillEllipse(Brush* brush, const Rect& rc) {
	FillEllipse(brush, (RectF)rc);
}

void Graphics::DrawEllipse(Pen* pen, const RectF& rc) {
	//XX
}

void Graphics::DrawArc(Pen* pen, const RectF& rc, float start, float sweep) {
	//XX
}

void Graphics::DrawPie(Pen* pen, const RectF& rc, float start, float sweep) {
	//XX
}

void Graphics::FillPie(Brush* brush, const RectF& rc, float start, float sweep) {
	//XX
}

void Graphics::DrawPolygon(Pen* pen, PointF* points, unsigned int pointCount) {
	//XX
}

void Graphics::DrawCurve(Pen* pen, PointF* points, unsigned int pointCount) {
	//XX
}

void Graphics::DrawEllipse(Pen* pen, const Rect& rc) {
	//XX
}

void Graphics::FillPolygon(Brush* brush, PointF* points, unsigned int pointCount, FillMode fm) {
	//XX
}

void Graphics::FillPath(Brush* brush, GraphicsPath* path) {
	//XX
}
void Graphics::DrawPath(Pen* pen, GraphicsPath* path) {
	//XX
}

void Graphics::DrawLine(Pen* pen, float xa, float ya, float xb, float yb) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	PenPrivate* pp = reinterpret_cast<PenPrivate*>(pen->_private);
	cairo_pattern_t* cp = pp->pattern;
	cairo_set_source(cg, cp);
	cairo_set_line_width(cg, pp->width);
	cairo_move_to(cg, xa, ya);
	cairo_line_to(cg, xb, yb);
	cairo_stroke(cg);
	cairo_restore(cg);
}

void Graphics::DrawLine(Pen* pen, int xa, int ya, int xb, int yb) {
	DrawLine(pen, (float)xa, (float)ya, (float)xb, (float)yb);
}

void Graphics::DrawLine(Pen* pen, const PointF& a, const PointF& b) {
	DrawLine(pen, a._x, a._y, b._x, b._y);
}

void Graphics::DrawLine(Pen* pen, const Point& a, const Point& b) {
	DrawLine(pen, (float)a._x, (float)a._y, (float)b._x, (float)b._y);
}

void Graphics::DrawString(const wchar_t* text, size_t length, Font* font, const RectF& area, const StringFormat* sf, Brush* brush) {
	/*if(length>0) {
		cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
		cairo_pattern_t* cp = reinterpret_cast<cairo_pattern_t*>(brush->_private);
		FontPrivate* fontPrivate = reinterpret_cast<FontPrivate*>(font->_private);
		
		char* utfFamily = ToUTF8(fontPrivate->family.c_str());
		char* utfText = ToUTF8(text);

		cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
		if((fontPrivate->style & FontStyleItalic)!=0) {
			slant = CAIRO_FONT_SLANT_ITALIC;
		}

		cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
		if((fontPrivate->style & FontStyleBold)!=0) {
			weight = CAIRO_FONT_WEIGHT_BOLD;
		}

		cairo_select_font_face(cg, utfFamily, slant, weight);
		cairo_set_font_size(cg, fontPrivate->size);
		cairo_rectangle(cg, area.GetLeft(), area.GetTop(), area.GetWidth(), area.GetHeight());
		cairo_clip(cg);
		cairo_move_to(cg, area.GetLeft(), area.GetTop());
		cairo_show_text(cg, utfText);
		delete[] utfFamily;
		delete[] utfText;
	}*/
}

void Graphics::DrawString(const wchar_t* text, size_t length, Font* font, const PointF& origin, const StringFormat* sf, Brush* brush) {
	//XX
}

void Graphics::DrawString(const wchar_t* text, size_t length, Font* font, const PointF& origin, Brush* brush) {
	//XX
}

void Graphics::MeasureString(const wchar_t* text,size_t length, const Font* font, const RectF& origin, const StringFormat* sf, RectF* boundingBox) {
	//XX
}

void Graphics::MeasureString(const wchar_t* text,size_t length, const Font* font, const PointF& origin, RectF* boundingBox) {
	//XX
}

void Graphics::MeasureString(const wchar_t* text,size_t length, const Font* font, const PointF& origin, const StringFormat* sf, RectF* boundingBox) {
	//XX
}

void Graphics::DrawImage(Image* image, int x, int y) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	cairo_surface_t* source = reinterpret_cast<cairo_surface_t*>(image->_private);
	cairo_set_source_surface(cg, source, 0.0, 0.0);
	int sourceWidth = cairo_image_surface_get_width(source);
	int sourceHeight = cairo_image_surface_get_height(source);
	cairo_rectangle(cg, x, y, sourceWidth, sourceHeight);
	cairo_fill(cg);
	cairo_restore(cg);
}

void Graphics::DrawImage(Image* image, int x, int y, int srcX, int srcY, int srcWidth, int srcHeight) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	cairo_surface_t* source = reinterpret_cast<cairo_surface_t*>(image->_private);
	cairo_set_source_surface(cg, source, srcX, srcY);
	cairo_rectangle(cg, x, y, srcWidth, srcHeight);
	cairo_fill(cg);
	cairo_restore(cg);
}

void Graphics::DrawImage(Image* image, const RectF& rc, const ImageAttributes* attr) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	cairo_surface_t* source = reinterpret_cast<cairo_surface_t*>(image->_private);

	cairo_pattern_t* cp = cairo_pattern_create_for_surface(source);
	int sourceWidth = cairo_image_surface_get_width(source);
	int sourceHeight = cairo_image_surface_get_height(source);

	cairo_matrix_t patternMatrix;
	cairo_matrix_init_scale(&patternMatrix, rc.GetWidth()/sourceWidth, rc.GetHeight()/sourceHeight);
	cairo_pattern_set_matrix(cp, &patternMatrix);

	cairo_pattern_set_extend(cp, CAIRO_EXTEND_REPEAT);

	cairo_set_source(cg, cp);
	cairo_rectangle(cg, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight());
	cairo_fill(cg);
	cairo_pattern_destroy(cp);
	cairo_restore(cg);
}

void Graphics::DrawImage(Image* image, const Rect& rc, const ImageAttributes* attr) {
	DrawImage(image, (RectF)rc, attr);
}

GraphicsContainer Graphics::BeginContainer() {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_save(cg);
	return 0;
}

void Graphics::EndContainer(const GraphicsContainer& gc) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_restore(cg);
}

void Graphics::TranslateTransform(float dx, float dy, MatrixOrder order) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_translate(cg, dx, dy);
	//XX matrixorder
}

void Graphics::ScaleTransform(float dx, float dy, MatrixOrder order) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_scale(cg, dx, dy);
	//XX matrixorder
}

void Graphics::RotateTransform(float angle, MatrixOrder order) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_rotate(cg, angle);
	//XX matrixorder
}

void Graphics::RotateAtTransform(const PointF& p, float angle, MatrixOrder order) {
	if(order==MatrixOrderPrepend) {
		TranslateTransform(p._x, p._y, order);
        RotateTransform(angle, order);
        TranslateTransform(-p._x, -p._y, order);
	}
	else {
		TranslateTransform(-p._x, -p._y, order);
		RotateTransform(angle, order);
		TranslateTransform(p._x, p._y, order);
	}
}

void Graphics::DrawRoundRectangle(Pen* pen, const RectF& rc, float d) {
	//XX
}

void Graphics::FillRoundRectangle(Brush* brush, const RectF& rc, float d) {
	//XX
}

void Graphics::ResetClip() {
	/*cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_reset_clip(cg);*/
}

void Graphics::SetClip(const RectF& rc) {
	/*cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_rectangle(cg, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight());
	cairo_clip(cg);*/
}

void Graphics::SetHighQuality(bool t) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	//XX
}

void Graphics::SetCompositingMode(CompositingMode cm) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	switch(cm) {
		default:
		case CompositingModeSourceOver:
			cairo_set_operator(cg, CAIRO_OPERATOR_OVER);
			break;

		case CompositingModeSourceCopy:
			cairo_set_operator(cg, CAIRO_OPERATOR_SOURCE);
			break;
	}
}

CompositingMode Graphics::GetCompositingMode() const {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_operator_t cm = cairo_get_operator(cg);
	switch(cm) {
		default:
		case CAIRO_OPERATOR_OVER:
			return CompositingModeSourceOver;

		case CAIRO_OPERATOR_SOURCE:
			return CompositingModeSourceCopy;
	}
}

void Graphics::Clear(const Color& col) {
	cairo_t* cg = reinterpret_cast<cairo_t*>(_private);
	cairo_set_source_rgba(cg, col._r, col._g, col._b, col._a);
	cairo_paint(cg);
}

#endif