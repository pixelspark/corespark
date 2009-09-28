#ifndef _TJCOLOR_H
#define _TJCOLOR_H

namespace tj {
	namespace shared {
		struct EXPORTED CMYKColor {
			CMYKColor(double c = 0.0, double m = 0.0, double y = 0.0, double k = 0.0);
			double _c, _m, _y, _k;
		};

		struct EXPORTED HSVColor {
			HSVColor(double h = 0.0, double s = 0.0, double v = 0.0);
			double _h, _s, _v;
		};

		class EXPORTED RGBColor: public virtual Object, public Serializable {
			public:
				inline RGBColor(): _a(0.0), _r(0.0), _g(0.0), _b(0.0) {}
				inline RGBColor(double aa, double ar, double ag, double ab): _a(aa), _r(ar), _g(ag), _b(ab) {}
				inline RGBColor(double r, double g, double b): _a(1.0), _r(r), _g(g), _b(b) {}
				inline RGBColor(unsigned char a, unsigned char r, unsigned char g, unsigned char b): _a(double(a)/255.0), _b(double(b)/255.0), _g(double(g)/255.0), _r(double(r)/255.0) {}
				inline RGBColor(unsigned char r, unsigned char g, unsigned char b): _a(1.0), _b(double(b)/255.0), _g(double(g)/255.0), _r(double(r)/255.0) {}
				inline RGBColor(int a, int r, int g, int b): _a(double(a)/255.0), _b(double(b)/255.0), _g(double(g)/255.0), _r(double(r)/255.0) {}
				inline RGBColor(int r, int g, int b): _a(1.0), _b(double(b)/255.0), _g(double(g)/255.0), _r(double(r)/255.0) {}
				
				virtual ~RGBColor();
				virtual void Save(TiXmlElement* parent);
				virtual void Load(TiXmlElement* you);

				inline unsigned char GetRed() const {
					return (unsigned char)(Util::Min(1.0, Util::Max(0.0, _r)) * 255.0);
				}

				inline unsigned char GetGreen() const {
					return (unsigned char)(Util::Min(1.0, Util::Max(0.0, _g)) * 255.0);
				}

				inline unsigned char GetBlue() const {
					return (unsigned char)(Util::Min(1.0, Util::Max(0.0, _b)) * 255.0);
				}

				inline unsigned char GetAlpha() const {
					return (unsigned char)(Util::Min(1.0, Util::Max(0.0, _a)) * 255.0);
				}

				inline bool operator==(const RGBColor& o) const {
					return o._a == _a && o._r == _r && o._g == _g && o._b == _b;
				}

				inline unsigned char GetR() const { return GetRed(); }
				inline unsigned char GetG() const { return GetGreen(); }
				inline unsigned char GetB() const { return GetBlue(); }
				inline unsigned char GetA() const { return GetAlpha(); }

				inline double GetValue() const {
					return _a + 10.0*_r + 100.0*_g + 1000.0*_b;
				}

				double _a;
				double _r;
				double _g;
				double _b;
		};

		EXPORTED std::wostream& operator<<(std::wostream& strm, const RGBColor& col);
		EXPORTED std::ostream& operator<<(std::ostream& strm, const RGBColor& col);

		class EXPORTED ColorSpaces {
			public:
				static RGBColor HSVToRGB(double h, double s, double v);
				static CMYKColor RGBToCMYK(double r, double g, double b);
				static HSVColor RGBToHSV(double r, double g, double b);
		};
	}
}

#endif
