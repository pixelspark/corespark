#ifndef _TJCOLOR_H
#define _TJCOLOR_H

namespace tj {
	namespace shared {
		struct EXPORTED CMYKColor {
			CMYKColor(double c, double m, double y, double k);
			double _c, _m, _y, _k;
		};

		struct EXPORTED HSVColor {
			HSVColor(double h, double s, double v);
			double _h, _s, _v;
		};

		struct EXPORTED RGBColor: public virtual Object, public Serializable {
			RGBColor(unsigned char ar=0, unsigned char ag=0, unsigned char ab=0) {
				r = ar;
				g = ag;
				b = ab;
			}

			virtual void Save(TiXmlElement* parent);
			virtual void Load(TiXmlElement* you);
			operator Gdiplus::Color();

			unsigned char r;
			unsigned char g;
			unsigned char b;
		};

		class EXPORTED ColorSpaces {
			public:
				static Gdiplus::Color HSVToRGB(double h, double s, double v);
				static CMYKColor RGBToCMYK(double r, double g, double b);
				static HSVColor RGBToHSV(double r, double g, double b);
		};
	}
}

#endif