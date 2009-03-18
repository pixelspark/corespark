#ifndef _TJRECTANGLE_H
#define _TJRECTANGLE_H

namespace tj {
	namespace shared {
		typedef int Pixels; // Logical pixels are 1/Theme::KDefaultDPI inches

		template<typename T> class BasicCoord {
			public:
				inline BasicCoord(const T& x = (T)0, const T& y = (T)0): _x(x), _y(y) {
				}

				template<typename Q> inline operator BasicCoord() {
					return BasicCoord<Q>((Q)_x, (Q)_y);
				}
			
			T _x;
			T _y;
		};

		typedef BasicCoord<Pixels> Coord;

		template<typename T> class BasicRectangle {
			public:
				inline BasicRectangle(const T& x = (T)0, const T& y = (T)0, const T& w = (T)0, const T& h = (T)0): _x(x), _y(y), _w(w), _h(h) {
				}

				#ifdef TJ_OS_WIN
					inline BasicRectangle(const RECT& r) {
						_x = T(r.left);
						_y = T(r.top);
						_h = T(r.bottom-r.top);
						_w = T(r.right-r.left);
					}
				#endif

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
					Narrow(-x, -y, -w, -h);
				}

				inline bool IsInside(T x, T y) const {
					return (x > _x && y > _y && x < (_x+_w) && y < (_y+_h));
				}
				
				inline bool IsInside(BasicCoord<T> pt) const {
					return IsInside(pt.x, pt.y);
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
					SaveAttributeSmall(parent,"x", _x);
					SaveAttributeSmall(parent,"y", _y);
					SaveAttributeSmall(parent,"w", _w);
					SaveAttributeSmall(parent,"h", _h);
				}

				inline void Load(TiXmlElement* you) {
					_x = LoadAttributeSmall(you, "x", _x);
					_y = LoadAttributeSmall(you, "y", _y);
					_w = LoadAttributeSmall(you, "w", _w);
					_h = LoadAttributeSmall(you, "h", _h);
				}

				template<typename Q> inline operator BasicRectangle<Q>() {
					return BasicRectangle<Q>((Q)_x, (Q)_y, (Q)_w, (Q)_h);
				}

			protected:
				T _x, _y, _w, _h;
		};

		typedef class BasicRectangle<Pixels> Area;
		typedef class BasicRectangle<float> AreaF;
	}
}

#endif