#ifndef _TJRECTANGLE_H
#define _TJRECTANGLE_H

namespace tj {
	namespace shared {
		template<typename T> class BasicRectangle {
			public:
				BasicRectangle(T x=0, T y=0, T w=0, T h=0) {
					_x = x;
					_y = y;
					_w = w;
					_h = h;
				}

				BasicRectangle(const RECT& r) {
					_x = T(r.left);
					_y = T(r.top);
					_h = T(r.bottom-r.top);
					_w = T(r.right-r.left);
				}

				BasicRectangle(const Gdiplus::RectF& r) {
					_x = (T)r.GetLeft();
					_y = (T)r.GetTop();
					_h = T(r.GetBottom())-_y;
					_w = T(r.GetRight())-_x;
				}

				virtual ~BasicRectangle() {
				}

				T GetX() const {
					return _x;
				}

				T GetY() const {
					return _y;
				}

				T GetWidth() const {
					return _w;
				}

				T GetHeight() const {
					return _h;
				}

				void SetX(T x) {
					_x = x;
				}

				void SetY(T y) {
					_y = y;
				}
				void SetWidth(T w) {
					_w = w;
				}

				void SetHeight(T h) {
					_h = h;
				}

				void Narrow(T x, T y, T w, T h) {
					_x += x;
					_w -= x;
					_y += y;
					_h -= y;
					_h -= h;
					_w -= w;
				}

				void Widen(T x, T y, T w, T h) {
					Narrow(-x, -y, -w, -h);
				}

				bool IsInside(T x, T y) const {
					return (x > _x && y > _y && x < (_x+_w) && y < (_y+_h));
				}
				
				inline operator Gdiplus::RectF() const {
					return Gdiplus::RectF(float(_x), float(_y), float(_w), float(_h));
				}

				inline operator RECT() const {
					RECT r;
					r.top = int(_y);
					r.bottom = int(_y + _h);
					r.left = int(_x);
					r.right = int(_x + _w);
					return r;
				}
				
				inline T GetTop() const {
					return GetY();
				}

				inline T GetLeft() const {
					return GetX();
				}

				inline T GetBottom() const {
					return GetY() + GetHeight();
				}

				inline T GetRight() const {
					return GetX() + GetWidth();
				}

				template<typename Q> void Multiply(Q wfactor, Q hfactor) {
					_w *= wfactor;
					_h *= hfactor;
					_x *= wfactor;
					_y *= hfactor;
				}

				template<typename Q> void Multiply(Q factor) {
					Multiply(factor,factor);
				}

				void Translate(T x, T y) {
					_x += x;
					_y += y;
				}

				template<typename Q> void MultiplyTranslate(Q wfactor, Q hfactor, T xoffset, T yoffset) {
					Multiply(wfactor,hfactor);
					Translate(xoffset, yoffset);
				}

				std::wstring ToString() const {
					std::wostringstream wos;
					wos << L"{";
					wos << _x << L",";
					wos << _y << L":";
					wos << _w << L",";
					wos << _h << L"}";
					return wos.str();
				}

			protected:
				T _x, _y, _w, _h;
		};

		typedef class BasicRectangle<int> Area;
		typedef class BasicRectangle<float> AreaF;
	}
}

#endif