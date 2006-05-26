#ifndef _TJRECTANGLE_H
#define _TJRECTANGLE_H

class EXPORTED Rectangle {
	public:
		Rectangle(int x=0, int y=0, int w=0, int h=0);
		Rectangle(const RECT& r);
		Rectangle(const Gdiplus::RectF& r);
		virtual ~Rectangle();
		int GetX() const;
		int GetY() const;
		int GetWidth() const;
		int GetHeight() const;
		void SetX(int x);
		void SetY(int y);
		void SetWidth(int w);
		void SetHeight(int h);
		void Narrow(int x, int y, int w, int h);
		void Widen(int x, int y, int w, int h);
		bool IsInside(int x, int y) const;
		
		inline operator Gdiplus::RectF() const {
			return Gdiplus::RectF(float(_x), float(_y), float(_w), float(_h));
		}

		inline operator RECT() const {
			RECT r;
			r.top = _y;
			r.bottom = _y + _h;
			r.left = _x;
			r.right = _x + _w;
			return r;
		}
		
		inline int GetTop() const {
			return GetY();
		}

		inline int GetLeft() const {
			return GetX();
		}

		inline int GetBottom() const {
			return GetY() + GetHeight();
		}

		inline int GetRight() const {
			return GetX() + GetWidth();
		}

	protected:
		int _x, _y, _w, _h;
};

#endif