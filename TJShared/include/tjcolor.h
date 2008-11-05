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

		struct EXPORTED RGBColor: public virtual Object, public Serializable {
			RGBColor(double ar = 0.0, double ag = 0.0, double ab = 0.0) {
				_r = ar;
				_g = ag;
				_b = ab;
			}

			virtual void Save(TiXmlElement* parent);
			virtual void Load(TiXmlElement* you);
			operator graphics::Color() const;

			double _r;
			double _g;
			double _b;
		};

		class EXPORTED ColorSpaces {
			public:
				static RGBColor HSVToRGB(double h, double s, double v);
				static CMYKColor RGBToCMYK(double r, double g, double b);
				static HSVColor RGBToHSV(double r, double g, double b);
		};
	}
}

#endif