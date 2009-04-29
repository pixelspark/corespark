#ifndef _TJRECTANGLE_H
#define _TJRECTANGLE_H

namespace tj {
	namespace shared {
		typedef int Pixels; // Logical pixels are 1/Theme::KDefaultDPI inches

		template<typename T, typename Storage = T> class BasicCoord {
			public:
				inline BasicCoord(const T& x = (T)0, const T& y = (T)0): _x(x), _y(y) {
				}

				template<typename Q> inline operator BasicCoord() {
					return BasicCoord<Q>((Q)_x, (Q)_y);
				}
			
			Storage _x;
			Storage _y;
		};

		typedef BasicCoord<Pixels,Pixels> Coord;

		template<typename T, typename Storage = T> class BasicRectangle {
			public:				
				template<typename Q> inline BasicRectangle(const Q& x, const Q& y, const Q& w, const Q& h): _x(x), _y(y), _w(w), _h(h) {
				}

				~BasicRectangle() {
				}

				inline T GetX() const {
					return _x;
				}

				inline T GetY() const {
					return _y;
				}

				inline T GetWidth() const {
					return _w;
				}

				inline T GetHeight() const {
					return _h;
				}

				inline void SetX(T x) {
					_x = x;
				}

				inline void SetY(T y) {
					_y = y;
				}
				
				inline void SetWidth(T w) {
					_w = w;
				}

				inline void SetHeight(T h) {
					_h = h;
				}

				inline void Narrow(T x, T y, T w, T h) {
					_x += x;
					_w -= x;
					_y += y;
					_h -= y;
					_h -= h;
					_w -= w;
				}

				inline void Widen(T x, T y, T w, T h) {
					_x -= x;
					_w += x;
					_y -= y;
					_h += y;
					_h += h;
					_w += w;
				}

				inline bool IsInside(T x, T y) const {
					return (x > _x && y > _y && x < (_x+_w) && y < (_y+_h));
				}
				
				template<typename Q> inline bool IsInside(BasicCoord<T,Q> pt) const {
					return IsInside((T)pt._x, (T)pt._y);
				}

				#ifdef TJ_OS_WIN
					inline operator RECT() const {
						RECT r;
						r.top = int(_y);
						r.bottom = int(_y + _h);
						r.left = int(_x);
						r.right = int(_x + _w);
						return r;
					}
				#endif
				
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

				template<typename Q> inline void Multiply(Q wfactor, Q hfactor) {
					_w = T(_w*wfactor);
					_h = T(_h*hfactor);
					_x = T(_x*wfactor);
					_y = T(_y*hfactor);
				}

				template<typename Q> inline void MultiplyCeil(Q wfactor, Q hfactor) {
					_w = (T)ceil(_w*wfactor);
					_h = (T)ceil(_h*hfactor);
					_x = (T)ceil(_x*wfactor);
					_y = (T)ceil(_y*hfactor);
				}

				template<typename Q> inline void Multiply(Q factor) {
					Multiply(factor,factor);
				}

				inline void Translate(T x, T y) {
					_x += x;
					_y += y;
				}

				template<typename Q> inline void MultiplyTranslate(Q wfactor, Q hfactor, T xoffset, T yoffset) {
					Multiply(wfactor,hfactor);
					Translate(xoffset, yoffset);
				}

				inline String ToString() const {
					std::wostringstream wos;
					wos << L"{";
					wos << _x << L",";
					wos << _y << L":";
					wos << _w << L",";
					wos << _h << L"}";
					return wos.str();
				}

				inline void Save(TiXmlElement* parent) const {
					SaveAttributeSmall<T>(parent,"x", _x);
					SaveAttributeSmall<T>(parent,"y", _y);
					SaveAttributeSmall<T>(parent,"w", _w);
					SaveAttributeSmall<T>(parent,"h", _h);
				}

				inline void Load(TiXmlElement* you) {
					_x = LoadAttributeSmall<T>(you, "x", _x);
					_y = LoadAttributeSmall<T>(you, "y", _y);
					_w = LoadAttributeSmall<T>(you, "w", _w);
					_h = LoadAttributeSmall<T>(you, "h", _h);
				}

				template<typename Q, typename R> inline operator BasicRectangle<Q,R>() {
					return BasicRectangle<Q,R>((Q)_x, (Q)_y, (Q)_w, (Q)_h);
				}

			protected:
				Storage _x, _y, _w, _h;
		};

		template<typename T> class SimpleRectangle: public BasicRectangle<T,T> {
			public:
				SimpleRectangle(): BasicRectangle<T,T>((T)0,(T)0,(T)0,(T)0) {
				}

				#ifdef TJ_OS_WIN
					inline SimpleRectangle(const RECT& r): BasicRectangle<T,T>((T)r.left, (T)r.top, (T)(r.right-r.left), (T)(r.bottom-r.top)) {
					}
				#endif

				SimpleRectangle(const T& x, const T& y, const T& w, const T& h): BasicRectangle<T,T>(x,y,w,h) {
				}

				~SimpleRectangle() {
				}

				template<typename Q> inline operator SimpleRectangle<Q>() {
					return SimpleRectangle<Q>((Q)BasicRectangle<T,T>::_x, (Q)BasicRectangle<T,T>::_y, (Q)BasicRectangle<T,T>::_w, (Q)BasicRectangle<T,T>::_h);
				}
		};

		typedef class SimpleRectangle<Pixels> Area;
		typedef class SimpleRectangle<float> AreaF;
	}
}

#endif