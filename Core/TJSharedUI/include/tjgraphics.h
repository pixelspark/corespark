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
 
 #ifndef _TJGRAPHICS_H
#define _TJGRAPHICS_H

namespace tj {
	namespace shared {
		namespace graphics {
			typedef tj::shared::SimpleRectangle<float> RectF;
			typedef tj::shared::SimpleRectangle<int> Rect;
			typedef tj::shared::BasicCoord<float> PointF;
			typedef tj::shared::BasicCoord<int> Point;
			typedef tj::shared::RGBColor Color;
			typedef unsigned int GraphicsContainer;

			// These constants should be the same as defined in GDI+
			typedef unsigned int StringFormatFlags;
			const static StringFormatFlags StringFormatFlagsNoWrap = 0x00001000;
			const static StringFormatFlags StringFormatFlagsLineLimit = 0x00002000;
			const static StringFormatFlags StringFormatFlagsNoClip = 0x00004000;

			typedef unsigned int FontStyle;
			const static FontStyle FontStyleRegular = 0x00000000;
			const static FontStyle FontStyleBold = 0x00000001;
			const static FontStyle FontStyleItalic = 0x00000002;
			const static FontStyle FontStyleBoldItalic = 0x00000003;
			const static FontStyle FontStyleUnderline = 0x00000004;
			const static FontStyle FontStyleStrikeout = 0x00000008;

			enum StringAlignment {
				StringAlignmentNear = 1,
				StringAlignmentFar,
				StringAlignmentCenter,
			};

			enum StringTrimming {
				StringTrimmingNone = 0,
				StringTrimmingCharacter,
				StringTrimmingWord,
				StringTrimmingEllipsisCharacter,
				StringTrimmingEllipsisWord,
				StringTrimmingEllipsisPath,
			};

			enum MatrixOrder {
				MatrixOrderPrepend = 0,
				MatrixOrderAppend,
			};

			enum WrapMode {
				WrapModeTile = 0,
				WrapModeClamp,
			};

			enum FillMode {
				FillModeWinding = 0,
				FillModeAlternate,
			};

			enum CompositingMode {
				CompositingModeSourceOver = 0,
				CompositingModeSourceCopy,
			};

			struct ColorMatrix {
				float m[5][5];
			};

			class EXPORTED StringFormat {
				friend class Graphics;

				public:
					StringFormat();
					~StringFormat();
					void SetAlignment(StringAlignment sa);
					void SetLineAlignment(StringAlignment sa);
					void SetTrimming(StringTrimming st);
					void SetFormatFlags(StringFormatFlags sf);
					StringFormatFlags GetFormatFlags() const;
					void SetTabStops(float firstTabOffset, unsigned int count, const float* tabStops);
					
				protected:
					void* _private;
			};

			/** Classes below are implemented differently per platform **/
			class EXPORTED Font {
				friend class Graphics;

				public:
					Font(const wchar_t* family, float pixelSize, FontStyle style);
					virtual ~Font();

				protected:
					void* _private;
			};

			class EXPORTED ImageAttributes {
				friend class Graphics;

				public:
					ImageAttributes();
					~ImageAttributes();
					void SetColorMatrix(const ColorMatrix* m);

				protected:
					void* _private;
			};


			class EXPORTED Brush {
				friend class Graphics;
				friend class Pen;

				public:
					virtual ~Brush();

				protected:
					void* _private;
			};

			class EXPORTED SolidBrush: public Brush {
				public:
					SolidBrush(const Color& color);
					virtual ~SolidBrush();
			};

			class EXPORTED LinearGradientBrush: public Brush {
				public:
					LinearGradientBrush(const PointF& a, const PointF& b, const Color& start, const Color& end);
					virtual ~LinearGradientBrush();
					void SetBlendPosition(float b);
			};

			class EXPORTED GraphicsPath {
				friend class PathGradientBrush;
				friend class Graphics;

				public:
					GraphicsPath();
					~GraphicsPath();
					void AddRectangle(const RectF& rc);
					void AddEllipse(const RectF& rc);
					void AddPolygon(const PointF* points, unsigned int pointCount);

				protected:
					void* _private;
			};

			class EXPORTED PathGradientBrush: public Brush {
				public:
					PathGradientBrush(GraphicsPath* gp);
					PathGradientBrush(PointF* points, unsigned int pointCount);
					virtual ~PathGradientBrush();
					void SetCenterColor(const Color& col);
					void SetCenterPoint(const PointF& point);
					void SetSurroundColors(Color* colors, unsigned int colorCount);
					void SetFocusScales(float fx, float fy);
			};

			class EXPORTED Pen {
				friend class Graphics;

				public:
					Pen(Brush* brush, float width = 1.0f);
					Pen(const Color& color, float width = 1.0f);
					Color GetColor() const;
					~Pen();

				protected:
					void* _private;
			};

			class EXPORTED Image {
				friend class Graphics;

				public:
					virtual ~Image();
					virtual int GetWidth();
					virtual int GetHeight();
					virtual bool Save(const wchar_t* path, const wchar_t* mime);

					static Image* FromFile(const wchar_t* path, bool useICM);

				protected:
					void* _private;
			};

			class EXPORTED BitmapData {
				friend class Bitmap;
				friend class Graphics;

				public:
					BitmapData();
					~BitmapData();
					unsigned char* GetPointer() const;
					unsigned int GetStride() const;
					
				protected:
					void* _private;
			};

			class EXPORTED Bitmap: public Image {
				friend class Graphics;
				friend class Image;

				public:
					Bitmap(unsigned int w, unsigned int h);
					Bitmap(unsigned int w, unsigned int h, Graphics* originalFormat);
					Bitmap(unsigned int w, unsigned int h, unsigned int pitch, unsigned char* data); // Only when data is in ARGB32 format
					virtual ~Bitmap();

					virtual bool LockBits(const Rect& rc, bool write, BitmapData* data);
					virtual void UnlockBits(BitmapData* data);

				protected:
					Bitmap();
			};

			class EXPORTED Graphics {
				friend class Bitmap;

				public:
					#ifdef WIN32
						Graphics(HDC dc);
						Graphics(HWND wnd);
					#endif

					Graphics(Image* bmp);
					~Graphics();

					void Clear(const Color& col);
					void FillRectangle(Brush* brush, const RectF& rc);
					void FillRectangle(Brush* brush, const Rect& rc);
					void DrawRectangle(Pen* pen, const RectF& rc);
					void DrawRectangle(Pen* pen, const Rect& rc);
					void DrawRoundRectangle(Pen* pen, const RectF& rc, float d);
					void FillRoundRectangle(Brush* brush, const RectF& rc, float d);

					void FillEllipse(Brush* brush, const RectF& rc);
					void FillEllipse(Brush* brush, const Rect& rc);
					void DrawEllipse(Pen* pen, const RectF& rc);
					void DrawEllipse(Pen* pen, const Rect& rc);
					void DrawArc(Pen* pen, const RectF& rc, float start, float sweep);
					void DrawCurve(Pen* pen, PointF* points, unsigned int pointCount);
					void FillPath(Brush* brush, GraphicsPath* path);
					void DrawPie(Pen* pen, const RectF& rc, float start, float sweep);
					void FillPie(Brush* pen, const RectF& rc, float start, float sweep);
					void DrawPath(Pen* pen, GraphicsPath* path);
					void DrawPolygon(Pen* pen, PointF* points, unsigned int pointCount);
					void FillPolygon(Brush* brush, PointF* points, unsigned int pointCount, FillMode fm = FillModeWinding);

					void DrawLine(Pen* pen, float xa, float ya, float xb, float yb);
					void DrawLine(Pen* pen, int xa, int ya, int xb, int yb);
					void DrawLine(Pen* pen, const PointF& a, const PointF& b);
					void DrawLine(Pen* pen, const Point& a, const Point& b);

					void DrawString(const wchar_t* text, size_t length, Font* font, const RectF& area, const StringFormat* sf, Brush* brush);
					void DrawString(const wchar_t* text, size_t length, Font* font, const PointF& origin, const StringFormat* sf, Brush* brush);
					void DrawString(const wchar_t* text, size_t length, Font* font, const PointF& origin, Brush* brush);
					void MeasureString(const wchar_t* text,size_t length, const Font* font, const RectF& lr, const StringFormat* sf, RectF* boundingBox);
					void MeasureString(const wchar_t* text,size_t length, const Font* font, const PointF& origin, RectF* boundingBox);
					void MeasureString(const wchar_t* text,size_t length, const Font* font, const PointF& origin, const StringFormat* sf, RectF* boundingBox);
					
					void DrawImage(Image* image, const RectF& rc, const ImageAttributes* attr = 0);
					void DrawImage(Image* image, const Rect& rc, const ImageAttributes* attr = 0);
					void DrawImage(Image* image, int x, int y);
					void DrawImage(Image* image, int x, int y, int srcX, int srcY, int srcWidth, int srcHeight);

					void TranslateTransform(float dx, float dy, MatrixOrder order = MatrixOrderPrepend);
					void ScaleTransform(float dx, float dy, MatrixOrder order = MatrixOrderPrepend);
					void RotateTransform(float angle, MatrixOrder order = MatrixOrderPrepend);
					void RotateAtTransform(const PointF& p, float angle, MatrixOrder order = MatrixOrderPrepend);

					GraphicsContainer BeginContainer();
					void EndContainer(const GraphicsContainer& gc);

					void ResetClip();
					void SetClip(const RectF& rc);

					void SetCompositingMode(CompositingMode cm);
					CompositingMode GetCompositingMode() const;
					void SetHighQuality(bool t);

				protected:
					void* _private;
			};

			class EXPORTED GraphicsInit {
				public:
					GraphicsInit();
					virtual ~GraphicsInit();
			};
		}
	}
}

#endif