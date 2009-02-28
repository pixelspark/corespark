#ifdef TJSHARED_USE_GDIPLUS
#ifdef WIN32
#include "../../include/ui/tjui.h"
#include "../../include/ui/tjgraphics.h"
#include <Gdiplus.h>
using namespace tj::shared::graphics;

inline void ToGDIColor(const Color& source, Gdiplus::Color& target) {
	target = Gdiplus::Color(source.GetAlpha(), source.GetRed(), source.GetGreen(), source.GetBlue());
}

template<typename P, typename GP> inline GP ToGDIPoint(const P& source) {
	GP target;
	target.X = source._x;
	target.Y = source._y;
	return target;
}

template<typename R, typename GR> inline GR ToGDIRect(const R& source) {
	GR target;
	target.X = source.GetLeft();
	target.Y = source.GetTop();
	target.Width = source.GetWidth();
	target.Height = source.GetHeight();
	return target;
}

int GetEncoderClsid(const wchar_t* format, CLSID* pClsid) {
   unsigned int num = 0;          // number of image encoders
   unsigned size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0) {
      return -1;  // Failure
   }

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL) {
	   return -1;  // Failure
   }

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for(unsigned int j = 0; j < num; ++j) {
      if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

Brush::~Brush() {
}

/** Pen **/
Pen::Pen(Brush* brush, float width) {
	_private = reinterpret_cast<void*>(new Gdiplus::Pen((Gdiplus::Brush*)brush->_private, width));
}

Pen::Pen(const Color& color, float width) {
	Gdiplus::Color gdiColor;
	ToGDIColor(color, gdiColor);
	_private = reinterpret_cast<void*>(new Gdiplus::Pen(gdiColor, width));
}

Pen::~Pen() {
	delete reinterpret_cast<Gdiplus::Pen*>(_private);
}

Color Pen::GetColor() const {
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(_private);
	Gdiplus::Color gdiColor;
	gdiPen->GetColor(&gdiColor);
	return Color(gdiColor.GetA(), gdiColor.GetR(), gdiColor.GetG(), gdiColor.GetB());
}

/** SolidBrush **/
SolidBrush::SolidBrush(const Color& col) {
	Gdiplus::Color gdiColor;
	ToGDIColor(col, gdiColor);
	_private = reinterpret_cast<void*>(dynamic_cast<Gdiplus::Brush*>(new Gdiplus::SolidBrush(gdiColor)));
}

SolidBrush::~SolidBrush() {
	delete reinterpret_cast<Gdiplus::SolidBrush*>(_private);
}

/** Font **/
Font::Font(const wchar_t* family, float pixelSize, FontStyle style) {
	_private = reinterpret_cast<void*>(new Gdiplus::Font(family, pixelSize, (Gdiplus::FontStyle)style, Gdiplus::UnitPixel, 0));
}
Font::~Font() {
	delete reinterpret_cast<Gdiplus::Font*>(_private);
}

/** Image **/
Image* Image::FromFile(const wchar_t* path, bool useICM) {
	Gdiplus::Image* gdiImage = Gdiplus::Image::FromFile(path, useICM);
	if(gdiImage) {
		Image* image = new Image();
		image->_private = reinterpret_cast<void*>(gdiImage);
		return image;
	}
	return 0;
}

Image::~Image() {
	delete reinterpret_cast<Gdiplus::Image*>(_private);
}

int Image::GetWidth() {
	Gdiplus::Image* gdiImage = reinterpret_cast<Gdiplus::Image*>(_private);
	return gdiImage->GetWidth();
}

int Image::GetHeight() {
	Gdiplus::Image* gdiImage = reinterpret_cast<Gdiplus::Image*>(_private);
	return gdiImage->GetHeight();
}

bool Image::Save(const wchar_t* path, const wchar_t* mime) {
	Gdiplus::Image* gdiBitmap = reinterpret_cast<Gdiplus::Image*>(_private);
	CLSID formatClsid;
	if(GetEncoderClsid(mime, &formatClsid)>0) {
		if(gdiBitmap->Save(path, &formatClsid, NULL)==Gdiplus::Ok) {
			return true;
		}
	}
	return false;
}

/** Bitmap **/
Bitmap::Bitmap(unsigned int w, unsigned int h) {
	Gdiplus::Bitmap* gdiBitmap = new Gdiplus::Bitmap(w, h);
	_private = reinterpret_cast<void*>(dynamic_cast<Gdiplus::Image*>(gdiBitmap));
}

Bitmap::Bitmap(unsigned int w, unsigned int h, Graphics* org) {
	Gdiplus::Graphics* gdiOriginal = reinterpret_cast<Gdiplus::Graphics*>(org->_private);
	Gdiplus::Bitmap* gdiBitmap = new Gdiplus::Bitmap(w, h, gdiOriginal);
	_private = reinterpret_cast<void*>(dynamic_cast<Gdiplus::Image*>(gdiBitmap));
}

Bitmap::Bitmap(unsigned int w, unsigned int h, unsigned int pitch, unsigned char* data) {
	Gdiplus::Bitmap* gdiBitmap = new Gdiplus::Bitmap(w, h, pitch, PixelFormat32bppARGB, data);
	_private = reinterpret_cast<void*>(dynamic_cast<Gdiplus::Image*>(gdiBitmap));
}

Bitmap::~Bitmap() {
	// _private is deleted in Image::~Image
}

bool Bitmap::LockBits(const Rect& rc, bool write, BitmapData* data) {
	Gdiplus::BitmapData* bd = reinterpret_cast<Gdiplus::BitmapData*>(data->_private);
	Gdiplus::Bitmap* gdiBitmap = dynamic_cast<Gdiplus::Bitmap*>(reinterpret_cast<Gdiplus::Image*>(_private));
	Gdiplus::Rect gdiRect(rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight());
	return gdiBitmap->LockBits(&gdiRect, write ? Gdiplus::ImageLockModeWrite : Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bd) == Gdiplus::Ok;
}

void Bitmap::UnlockBits(BitmapData* data) {
	Gdiplus::BitmapData* bd = reinterpret_cast<Gdiplus::BitmapData*>(data->_private);
	Gdiplus::Bitmap* gdiBitmap = dynamic_cast<Gdiplus::Bitmap*>(reinterpret_cast<Gdiplus::Image*>(_private));
	gdiBitmap->UnlockBits(bd);
}

/** LinearGradientBrush **/
LinearGradientBrush::LinearGradientBrush(const PointF& a, const PointF& b, const Color& start, const Color& end) {
	Gdiplus::Color gdiStart, gdiEnd;
	ToGDIColor(start, gdiStart);
	ToGDIColor(end, gdiEnd);
	Gdiplus::LinearGradientBrush* gdiBrush = new Gdiplus::LinearGradientBrush(ToGDIPoint<PointF, Gdiplus::PointF>(a), ToGDIPoint<PointF, Gdiplus::PointF>(b), gdiStart, gdiEnd);
	_private = reinterpret_cast<void*>(dynamic_cast<Gdiplus::Brush*>(gdiBrush));
}

LinearGradientBrush::~LinearGradientBrush() {
	delete reinterpret_cast<Gdiplus::Brush*>(_private);
}

void LinearGradientBrush::SetBlendPosition(float b) {
	Gdiplus::LinearGradientBrush* gdiBrush = dynamic_cast<Gdiplus::LinearGradientBrush*>(reinterpret_cast<Gdiplus::Brush*>(_private));
	float blendPositions[3] = { 0.0f, b, 1.0f };
	float blendFactors[3] = { 0.0f, 0.5f, 1.0f };
	gdiBrush->SetBlend(blendFactors, blendPositions, 3);
}

/** Graphics **/
Graphics::Graphics(Image* bmp) {
	Gdiplus::Graphics* g = new Gdiplus::Graphics(reinterpret_cast<Gdiplus::Image*>(bmp->_private));
	/*g->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	g->SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);*/
	_private = reinterpret_cast<void*>(g);	
}

Graphics::Graphics(HWND wnd) {
	Gdiplus::Graphics* g = new Gdiplus::Graphics(wnd);
	g->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	g->SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
	_private = reinterpret_cast<void*>(g);
}

Graphics::Graphics(HDC dc) {
	Gdiplus::Graphics* g = new Gdiplus::Graphics(dc);
	g->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	g->SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
	_private = reinterpret_cast<void*>(g);
}

Graphics::~Graphics() {
	delete reinterpret_cast<Gdiplus::Graphics*>(_private);
}

void Graphics::FillRectangle(Brush* brush, const RectF& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	g->FillRectangle(gdiBrush, ToGDIRect<RectF, Gdiplus::RectF>(rc));
}

void Graphics::FillRectangle(Brush* brush, const Rect& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	g->FillRectangle(gdiBrush, ToGDIRect<Rect, Gdiplus::Rect>(rc));
}

void Graphics::DrawRectangle(Pen* pen, const RectF& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	g->DrawRectangle(gdiPen, ToGDIRect<RectF, Gdiplus::RectF>(rc));
}
void Graphics::DrawRectangle(Pen* pen, const Rect& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	g->DrawRectangle(gdiPen, ToGDIRect<Rect, Gdiplus::Rect>(rc));
}

void Graphics::FillEllipse(Brush* brush, const RectF& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	g->FillEllipse(gdiBrush, ToGDIRect<RectF, Gdiplus::RectF>(rc));
}

void Graphics::FillEllipse(Brush* brush, const Rect& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	g->FillEllipse(gdiBrush, ToGDIRect<Rect, Gdiplus::Rect>(rc));
}

void Graphics::DrawEllipse(Pen* pen, const RectF& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	g->DrawEllipse(gdiPen, ToGDIRect<RectF, Gdiplus::RectF>(rc));
}

void Graphics::DrawArc(Pen* pen, const RectF& rc, float start, float sweep) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	g->DrawArc(gdiPen, Gdiplus::RectF(rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight()), start, sweep);
}

void Graphics::DrawPie(Pen* pen, const RectF& rc, float start, float sweep) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	g->DrawPie(gdiPen, Gdiplus::RectF(rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight()), start, sweep);
}

void Graphics::FillPie(Brush* brush, const RectF& rc, float start, float sweep) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	g->FillPie(gdiBrush, Gdiplus::RectF(rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight()), start, sweep);
}

void Graphics::DrawPolygon(Pen* pen, PointF* points, unsigned int pointCount) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);

	Gdiplus::PointF* gdiPoints = new Gdiplus::PointF[pointCount];
	for(unsigned int a = 0; a < pointCount; a++) {
		gdiPoints[a] = Gdiplus::PointF(points[a]._x, points[a]._y);
	}

	g->DrawPolygon(gdiPen, gdiPoints, pointCount);
	delete[] gdiPoints;
}

void Graphics::DrawCurve(Pen* pen, PointF* points, unsigned int pointCount) {
	Gdiplus::PointF* gdiPoints = new Gdiplus::PointF[pointCount];
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	
	for(unsigned int a=0; a<pointCount; a++) {
		gdiPoints[a] = Gdiplus::PointF(points[a]._x, points[a]._y);
	}
	g->DrawCurve(gdiPen, gdiPoints, pointCount);
	delete[] gdiPoints;
}

void Graphics::DrawEllipse(Pen* pen, const Rect& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	g->DrawEllipse(gdiPen, ToGDIRect<Rect, Gdiplus::Rect>(rc));
}

void Graphics::FillPolygon(Brush* brush, PointF* points, unsigned int pointCount, FillMode fm) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);

	Gdiplus::PointF* gdiPoints = new Gdiplus::PointF[pointCount];
	for(unsigned int a = 0; a < pointCount; a++) {
		gdiPoints[a] = Gdiplus::PointF(points[a]._x, points[a]._y);
	}

	g->FillPolygon(gdiBrush, gdiPoints, pointCount, fm == FillModeWinding ? Gdiplus::FillModeWinding : Gdiplus::FillModeAlternate);
	delete[] gdiPoints;
}

void Graphics::FillPath(Brush* brush, GraphicsPath* path) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	Gdiplus::GraphicsPath* gdiPath = reinterpret_cast<Gdiplus::GraphicsPath*>(path->_private);
	g->FillPath(gdiBrush, gdiPath);
}
void Graphics::DrawPath(Pen* pen, GraphicsPath* path) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	Gdiplus::GraphicsPath* gdiPath = reinterpret_cast<Gdiplus::GraphicsPath*>(path->_private);
	g->DrawPath(gdiPen, gdiPath);
}

void Graphics::DrawLine(Pen* pen, float xa, float ya, float xb, float yb) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	g->DrawLine(gdiPen, xa, ya, xb, yb);
}

void Graphics::DrawLine(Pen* pen, int xa, int ya, int xb, int yb) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);
	g->DrawLine(gdiPen, xa, ya, xb, yb);
}

void Graphics::DrawLine(Pen* pen, const PointF& a, const PointF& b) {
	DrawLine(pen, a._x, a._y, b._x, b._y);
}

void Graphics::DrawLine(Pen* pen, const Point& a, const Point& b) {
	DrawLine(pen, a._x, a._y, b._x, b._y);
}

void Graphics::DrawString(const wchar_t* text, size_t length, Font* font, const RectF& area, const StringFormat* sf, Brush* brush) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	Gdiplus::Font* gdiFont = reinterpret_cast<Gdiplus::Font*>(font->_private);
	Gdiplus::StringFormat* gdiFormat = reinterpret_cast<Gdiplus::StringFormat*>(sf->_private);
	g->DrawString(text, (int)length, gdiFont, ToGDIRect<RectF, Gdiplus::RectF>(area), gdiFormat, gdiBrush);
}

void Graphics::DrawString(const wchar_t* text, size_t length, Font* font, const PointF& origin, const StringFormat* sf, Brush* brush) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	Gdiplus::Font* gdiFont = reinterpret_cast<Gdiplus::Font*>(font->_private);
	Gdiplus::StringFormat* gdiFormat = reinterpret_cast<Gdiplus::StringFormat*>(sf->_private);
	g->DrawString(text, (int)length, gdiFont, ToGDIPoint<PointF, Gdiplus::PointF>(origin), gdiFormat, gdiBrush);
}


void Graphics::DrawString(const wchar_t* text, size_t length, Font* font, const PointF& origin, Brush* brush) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);
	Gdiplus::Font* gdiFont = reinterpret_cast<Gdiplus::Font*>(font->_private);
	g->DrawString(text, (int)length, gdiFont, ToGDIPoint<PointF, Gdiplus::PointF>(origin), gdiBrush);
}

void Graphics::MeasureString(const wchar_t* text,size_t length, const Font* font, const RectF& origin, const StringFormat* sf, RectF* boundingBox) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Font* gdiFont = reinterpret_cast<Gdiplus::Font*>(font->_private);
	Gdiplus::StringFormat* gdiFormat = reinterpret_cast<Gdiplus::StringFormat*>(sf->_private);

	Gdiplus::RectF gdiBound;
	g->MeasureString(text, (int)length, gdiFont, ToGDIRect<RectF, Gdiplus::RectF>(origin), gdiFormat, &gdiBound);
	*boundingBox = RectF(gdiBound.X, gdiBound.Y, gdiBound.Width, gdiBound.Height);
}

void Graphics::MeasureString(const wchar_t* text,size_t length, const Font* font, const PointF& origin, RectF* boundingBox) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Font* gdiFont = reinterpret_cast<Gdiplus::Font*>(font->_private);

	Gdiplus::RectF gdiBound;
	g->MeasureString(text, (int)length, gdiFont, ToGDIPoint<PointF, Gdiplus::PointF>(origin), &gdiBound);
	*boundingBox = RectF(gdiBound.X, gdiBound.Y, gdiBound.Width, gdiBound.Height);
}

void Graphics::MeasureString(const wchar_t* text,size_t length, const Font* font, const PointF& origin, const StringFormat* sf, RectF* boundingBox) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Font* gdiFont = reinterpret_cast<Gdiplus::Font*>(font->_private);
	Gdiplus::StringFormat* gdiFormat = reinterpret_cast<Gdiplus::StringFormat*>(sf->_private);

	Gdiplus::RectF gdiBound;
	g->MeasureString(text, (int)length, gdiFont, ToGDIPoint<PointF, Gdiplus::PointF>(origin), gdiFormat, &gdiBound);
	*boundingBox = RectF(gdiBound.X, gdiBound.Y, gdiBound.Width, gdiBound.Height);
}

void Graphics::DrawImage(Image* image, int x, int y) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Image* gdiImage = reinterpret_cast<Gdiplus::Image*>(image->_private);
	g->DrawImage(gdiImage, x, y);
}

void Graphics::DrawImage(Image* image, const RectF& rc, const ImageAttributes* attr) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Image* gdiImage = reinterpret_cast<Gdiplus::Image*>(image->_private);

	if(attr!=0) {
		Gdiplus::ImageAttributes* ia = reinterpret_cast<Gdiplus::ImageAttributes*>(attr->_private);
		g->DrawImage(gdiImage, Gdiplus::RectF(rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight()), 0.0f, 0.0f, (float)gdiImage->GetWidth(), (float)gdiImage->GetHeight(), Gdiplus::UnitPixel, ia);
	}
	else {
		g->DrawImage(gdiImage, ToGDIRect<RectF, Gdiplus::RectF>(rc));
	}
}

void Graphics::DrawImage(Image* image, const Rect& rc, const ImageAttributes* attr) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Image* gdiImage = reinterpret_cast<Gdiplus::Image*>(image->_private);
	
	if(attr!=0) {
		Gdiplus::ImageAttributes* ia = reinterpret_cast<Gdiplus::ImageAttributes*>(attr->_private);
		g->DrawImage(gdiImage, Gdiplus::Rect(rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight()), 0, 0, gdiImage->GetWidth(), gdiImage->GetHeight(), Gdiplus::UnitPixel, ia);
	}
	else {
		g->DrawImage(gdiImage, ToGDIRect<Rect, Gdiplus::Rect>(rc));
	}
}

void Graphics::DrawImage(Image* image, int x, int y, int srcX, int srcY, int srcWidth, int srcHeight) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Image* gdiImage = reinterpret_cast<Gdiplus::Image*>(image->_private);
	g->DrawImage(gdiImage, x, y, srcX, srcY, srcWidth, srcHeight, Gdiplus::UnitPixel);
}

GraphicsContainer Graphics::BeginContainer() {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	GraphicsContainer gc = static_cast<GraphicsContainer>(g->BeginContainer());
	/*g->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	g->SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);*/
	return gc;
}

void Graphics::EndContainer(const GraphicsContainer& gc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	g->EndContainer(static_cast<Gdiplus::GraphicsContainer>(gc));
}

void Graphics::TranslateTransform(float dx, float dy, MatrixOrder order) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	g->TranslateTransform(dx, dy, order == MatrixOrderPrepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
}

void Graphics::ScaleTransform(float dx, float dy, MatrixOrder order) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	g->ScaleTransform(dx, dy, order == MatrixOrderPrepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
}

void Graphics::RotateTransform(float angle, MatrixOrder order) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	g->RotateTransform(angle, order == MatrixOrderPrepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
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
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Pen* gdiPen = reinterpret_cast<Gdiplus::Pen*>(pen->_private);

	Gdiplus::GraphicsPath gp;
	Gdiplus::RectF r(rc.GetLeft()-1.0f, rc.GetTop()-1.0f, rc.GetWidth(), rc.GetHeight());

	gp.AddArc(r.X, r.Y, d, d, 180.0f, 90.0f);
	gp.AddArc(r.X + r.Width - d, r.Y, d, d, 270.0f, 90.0f);
	gp.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0.0f, 90.0f);
	gp.AddArc(r.X, r.Y + r.Height - d, d, d, 90.0f, 90.0f);
	gp.AddLine(r.X, r.Y + r.Height - d, r.X, r.Y + d / 2.0f);
	
	g->DrawPath(gdiPen, &gp);
}

void Graphics::FillRoundRectangle(Brush* brush, const RectF& rc, float d) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::Brush* gdiBrush = reinterpret_cast<Gdiplus::Brush*>(brush->_private);

	Gdiplus::GraphicsPath gp;
	Gdiplus::RectF r(rc.GetLeft()-1.0f, rc.GetTop()-1.0f, rc.GetWidth(), rc.GetHeight());

	gp.AddArc(r.X, r.Y, d, d, 180.0f, 90.0f);
	gp.AddArc(r.X + r.Width - d, r.Y, d, d, 270.0f, 90.0f);
	gp.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0.0f, 90.0f);
	gp.AddArc(r.X, r.Y + r.Height - d, d, d, 90.0f, 90.0f);
	gp.AddLine(r.X, r.Y + r.Height - d, r.X, r.Y + d / 2.0f);
	
	g->FillPath(gdiBrush, &gp);
}

void Graphics::ResetClip() {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	g->ResetClip();
}

void Graphics::SetClip(const RectF& rc) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	g->SetClip(ToGDIRect<RectF, Gdiplus::RectF>(rc));
}

void Graphics::SetHighQuality(bool t) {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	if(t) {
		g->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		g->SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
	}
	else {
		g->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		g->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
	}
}

void Graphics::SetCompositingMode(CompositingMode cm) {
	Gdiplus::CompositingMode gdiCm;
	switch(cm) {
		case CompositingModeSourceCopy:
			gdiCm = Gdiplus::CompositingModeSourceCopy;
			break;

		case CompositingModeSourceOver:
		default:
			gdiCm = Gdiplus::CompositingModeSourceOver;
			break;
	}

	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	g->SetCompositingMode(gdiCm);
}

CompositingMode Graphics::GetCompositingMode() const {
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	Gdiplus::CompositingMode gdiCm = g->GetCompositingMode();
	switch(gdiCm) {
		case Gdiplus::CompositingModeSourceCopy:
			return CompositingModeSourceCopy;

		case Gdiplus::CompositingModeSourceOver:
		default:
			return CompositingModeSourceOver;
	}
}

void Graphics::Clear(const Color& col) {
	Gdiplus::Color gdiColor;
	ToGDIColor(col, gdiColor);
	Gdiplus::Graphics* g = reinterpret_cast<Gdiplus::Graphics*>(_private);
	g->Clear(gdiColor);
}

/** GraphicsPath **/
GraphicsPath::GraphicsPath() {
	_private = reinterpret_cast<void*>(new Gdiplus::GraphicsPath());
}

GraphicsPath::~GraphicsPath() {
	delete reinterpret_cast<Gdiplus::GraphicsPath*>(_private);
}

void GraphicsPath::AddRectangle(const RectF& rc) {
	Gdiplus::GraphicsPath* gp = reinterpret_cast<Gdiplus::GraphicsPath*>(_private);
	gp->AddRectangle(ToGDIRect<RectF, Gdiplus::RectF>(rc));
}

void GraphicsPath::AddEllipse(const RectF& rc) {
	Gdiplus::GraphicsPath* gp = reinterpret_cast<Gdiplus::GraphicsPath*>(_private);
	gp->AddEllipse(ToGDIRect<RectF, Gdiplus::RectF>(rc));
}

void GraphicsPath::AddPolygon(const PointF* points, unsigned int pointCount) {
	Gdiplus::GraphicsPath* gp = reinterpret_cast<Gdiplus::GraphicsPath*>(_private);
	Gdiplus::PointF* gdiPoints = new Gdiplus::PointF[pointCount];
	for(unsigned int a=0; a<pointCount; a++) {
		gdiPoints[a] = Gdiplus::PointF(points[a]._x, points[a]._y);
	}
	gp->AddPolygon(gdiPoints, pointCount);
	delete[] gdiPoints;
}

/** GraphicsInit **/
GraphicsInit::GraphicsInit() {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

GraphicsInit::~GraphicsInit() {
}

/** StringFormat **/
StringFormat::StringFormat() {
	_private = reinterpret_cast<void*>(new Gdiplus::StringFormat());
}

StringFormat::~StringFormat() {
	delete reinterpret_cast<Gdiplus::StringFormat*>(_private);
}

void StringFormat::SetAlignment(StringAlignment sa) {
	Gdiplus::StringFormat* sf =  reinterpret_cast<Gdiplus::StringFormat*>(_private);
	switch(sa) {
		case StringAlignmentNear:
			sf->SetAlignment(Gdiplus::StringAlignmentNear);
			break;

		case StringAlignmentFar:
			sf->SetAlignment(Gdiplus::StringAlignmentFar);
			break;

		case StringAlignmentCenter:
			sf->SetAlignment(Gdiplus::StringAlignmentCenter);
			break;
	}
}

void StringFormat::SetLineAlignment(StringAlignment sa) {
	Gdiplus::StringFormat* sf =  reinterpret_cast<Gdiplus::StringFormat*>(_private);
	switch(sa) {
		case StringAlignmentNear:
			sf->SetLineAlignment(Gdiplus::StringAlignmentNear);
			break;

		case StringAlignmentFar:
			sf->SetLineAlignment(Gdiplus::StringAlignmentFar);
			break;

		case StringAlignmentCenter:
			sf->SetLineAlignment(Gdiplus::StringAlignmentCenter);
			break;
	}
}

void StringFormat::SetTrimming(StringTrimming st) {
	Gdiplus::StringFormat* sf =  reinterpret_cast<Gdiplus::StringFormat*>(_private);
	switch(st) {
		case StringTrimmingNone:
			sf->SetTrimming(Gdiplus::StringTrimmingNone);
			break;

		case StringTrimmingCharacter:
			sf->SetTrimming(Gdiplus::StringTrimmingCharacter);
			break;

		case StringTrimmingWord:
			sf->SetTrimming(Gdiplus::StringTrimmingWord);
			break;

		case StringTrimmingEllipsisCharacter:
			sf->SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
			break;

		case StringTrimmingEllipsisWord:
			sf->SetTrimming(Gdiplus::StringTrimmingEllipsisWord);
			break;

		case StringTrimmingEllipsisPath:
			sf->SetTrimming(Gdiplus::StringTrimmingEllipsisPath);
			break;
	}
}

void StringFormat::SetFormatFlags(StringFormatFlags sf) {
	Gdiplus::StringFormat* gdiSf =  reinterpret_cast<Gdiplus::StringFormat*>(_private);
	gdiSf->SetFormatFlags(sf);
}

StringFormatFlags StringFormat::GetFormatFlags() const {
	Gdiplus::StringFormat* sf =  reinterpret_cast<Gdiplus::StringFormat*>(_private);
	return sf->GetFormatFlags();
}

void StringFormat::SetTabStops(float firstTabOffset, unsigned int count, const float* tabStops) {
}

/** ImageAttributes **/
ImageAttributes::ImageAttributes() {
	_private = reinterpret_cast<void*>(new Gdiplus::ImageAttributes());
}

ImageAttributes::~ImageAttributes() {
	delete reinterpret_cast<Gdiplus::ImageAttributes*>(_private);
}

void ImageAttributes::SetColorMatrix(const ColorMatrix* cm) {
	Gdiplus::ColorMatrix gdiMatrix;
	for(int a = 0; a < 5; a++) {
		for(int b = 0; b < 5; b++) {
			gdiMatrix.m[a][b] = cm->m[a][b];
		}
	}

	Gdiplus::ImageAttributes* ia = reinterpret_cast<Gdiplus::ImageAttributes*>(_private);
	ia->SetColorMatrix(&gdiMatrix);
}

/** PathGradientBrush **/
PathGradientBrush::PathGradientBrush(PointF* points, unsigned int pointCount) {
	Gdiplus::PointF* gdiPoints = new Gdiplus::PointF[pointCount];
	for(unsigned int a=0; a<pointCount; a++) {
		 gdiPoints[a] = ToGDIPoint<PointF, Gdiplus::PointF>(points[a]);
	}
	Gdiplus::PathGradientBrush* pg = new Gdiplus::PathGradientBrush(gdiPoints, pointCount);
	_private = reinterpret_cast<void*>(dynamic_cast<Gdiplus::Brush*>(pg));
	delete[] gdiPoints;
}

PathGradientBrush::PathGradientBrush(GraphicsPath* gp) {
	Gdiplus::GraphicsPath* gdiPath = reinterpret_cast<Gdiplus::GraphicsPath*>(gp->_private);
	Gdiplus::PathGradientBrush* pg = new Gdiplus::PathGradientBrush(gdiPath);
	_private = reinterpret_cast<void*>(dynamic_cast<Gdiplus::Brush*>(pg));
}

PathGradientBrush::~PathGradientBrush() {
	delete reinterpret_cast<Gdiplus::Brush*>(_private);
}

void PathGradientBrush::SetFocusScales(float fx, float fy) {
	Gdiplus::PathGradientBrush* gdiBrush = dynamic_cast<Gdiplus::PathGradientBrush*>(reinterpret_cast<Gdiplus::Brush*>(_private));
	gdiBrush->SetFocusScales(fx,fy);
}

void PathGradientBrush::SetCenterColor(const Color& col) {
	Gdiplus::PathGradientBrush* gdiBrush = dynamic_cast<Gdiplus::PathGradientBrush*>(reinterpret_cast<Gdiplus::Brush*>(_private));
	Gdiplus::Color gdiColor;
	ToGDIColor(col, gdiColor);
	gdiBrush->SetCenterColor(gdiColor);
}

void PathGradientBrush::SetCenterPoint(const PointF& point) {
	Gdiplus::PathGradientBrush* gdiBrush = dynamic_cast<Gdiplus::PathGradientBrush*>(reinterpret_cast<Gdiplus::Brush*>(_private));
	Gdiplus::PointF gdiPoint = ToGDIPoint<PointF, Gdiplus::PointF>(point);
	gdiBrush->SetCenterPoint(gdiPoint);
}

void PathGradientBrush::SetSurroundColors(Color* colors, unsigned int colorCount) {
	Gdiplus::PathGradientBrush* gdiBrush = dynamic_cast<Gdiplus::PathGradientBrush*>(reinterpret_cast<Gdiplus::Brush*>(_private));
	Gdiplus::Color* gdiColors = new Gdiplus::Color[colorCount];
	for(unsigned int a=0; a<colorCount; a++) {
		ToGDIColor(colors[a], gdiColors[a]);
	}
	int count = colorCount;
	gdiBrush->SetSurroundColors(gdiColors, &count);
	delete[] gdiColors;
}

/** BitmapData **/
BitmapData::BitmapData() {
	_private = reinterpret_cast<void*>(new Gdiplus::BitmapData());
}

BitmapData::~BitmapData() {
	delete reinterpret_cast<Gdiplus::BitmapData*>(_private);
}

unsigned char* BitmapData::GetPointer() const {
	Gdiplus::BitmapData* bd = reinterpret_cast<Gdiplus::BitmapData*>(_private);
	return reinterpret_cast<unsigned char*>(bd->Scan0);
}

unsigned int BitmapData::GetStride() const {
	Gdiplus::BitmapData* bd = reinterpret_cast<Gdiplus::BitmapData*>(_private);
	return bd->Stride;
}

#endif
#endif