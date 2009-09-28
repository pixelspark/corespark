#include "../include/tjshared.h"
using namespace tj::shared;

/** Black   = minimum(1-Red,1-Green,1-Blue)
    Cyan    = (1-Red-Black)/(1-Black)
    Magenta = (1-Green-Black)/(1-Black)
    Yellow  = (1-Blue-Black)/(1-Black) */

CMYKColor ColorSpaces::RGBToCMYK(double r, double g, double b) {
	CMYKColor cmyk(0.0, 0.0, 0.0, 0.0);

	cmyk._k = Util::Min(1.0-r, Util::Min(1.0-g, 1.0-b));
	if(cmyk._k<1.0) {
		cmyk._c = (1.0-r-cmyk._k)/(1.0-cmyk._k);
		cmyk._m = (1.0-g-cmyk._k)/(1.0-cmyk._k);
		cmyk._y = (1.0-b-cmyk._k)/(1.0-cmyk._k);
	}

	return cmyk;
}

CMYKColor::CMYKColor(double c, double m, double y, double k): _c(c), _m(m), _y(y), _k(k) {
}

HSVColor::HSVColor(double h, double s, double v): _h(h), _s(s), _v(v) {
}

HSVColor ColorSpaces::RGBToHSV(double r, double g, double b) {
	// In this function, R, G, and B values must be scaled 
	// to be between 0 and 1.
	// HSV.Hue will be a value between 0 and 360, and 
	// HSV.Saturation and value are between 0 and 1.
	// The code must scale these to be between 0 and 255 for
	// the purposes of this application.
	double h;
	double s;

	double dmin = Util::Min(Util::Min(r, g), b);
	double dmax = Util::Max(Util::Max(r, g), b);
	double v = dmax;
	double delta = dmax - dmin;

	if(dmax == 0.0 || delta == 0.0) {
		// R, G, and B must be 0, or all the same.
		// In this case, S is 0, and H is undefined.
		// Using H = 0 is as good as any...
		s = 0.0;
		h = 0.0;
	} 
	else {
		s = delta / dmax;
		if(r == dmax) {
			// Between Yellow and Magenta
			h = (g - b) / delta;
		} 
		else if(g == dmax) {
			// Between Cyan and Yellow
			h = 2 + (b - r) / delta;
		} 
		else {
			// Between Magenta and Cyan
			h = 4 + (r - g) / delta;
		}
	}

	// Scale h to be between 0 and 360. 
	// This may require adding 360, if the value
	// is negative.
	h *= 60.0;
	if (h < 0) {
		h += 360.0;
	}

	// Scale to the requirements of this 
	// application. All values are between 0 and 255.
	return HSVColor(h/360.0,s,v);
}

// Hue (h), Saturation (s) and Value (v) between 0 and 1
RGBColor ColorSpaces::HSVToRGB(double h, double s, double v)  {
	double r,g,b;
	r = g = b = 0.0;

	// Scale Hue to be between 0 and 360. Saturation
	// and value scale to be between 0 and 1.
	h = fmod(h*360.0, 360.0);

	if(s == 0.0) {
		// Gray shade
		return RGBColor(v,v,v);
	} 
	else {
		// The color wheel consists of 6 sectors.
		// Figure out which sector you're in.
		double sectorPos = h / 60.0;
		int sectorNumber = int(floor(sectorPos));

		// get the fractional part of the sector.
		// That is, how many degrees into the sector
		// are you?
		double fractionalSector = sectorPos - double(sectorNumber);

		// Calculate values for the three axes
		// of the color. 
		double p = v * (1.0 - s);
		double q = v * (1.0 - (s * fractionalSector));
		double t = v * (1.0 - (s * (1.0 - fractionalSector)));

		// Assign the fractional colors to r, g, and b
		// based on the sector the angle is in.
		switch (sectorNumber) {
			case 0:
				r = v;
				g = t;
				b = p;
				break;

			case 1:
				r = q;
				g = v;
				b = p;
				break;

			case 2:
				r = p;
				g = v;
				b = t;
				break;

			case 3:
				r = p;
				g = q;
				b = v;
				break;

			case 4:
				r = t;
				g = p;
				b = v;
				break;

			case 5:
				r = v;
				g = p;
				b = q;
				break;
		}

		return RGBColor(r,g,b);
	}
}


/* RGBColor */
void RGBColor::Save(TiXmlElement* parent) {
	SaveAttributeSmall(parent,"r", (int)(_r*255.0));
	SaveAttributeSmall(parent, "g", (int)(_g*255.0));
	SaveAttributeSmall(parent, "b", (int)(_b*255.0));
}

void RGBColor::Load(TiXmlElement* you) {
	_r = double(LoadAttributeSmall(you, "r", (int)(_r*255.0))) / 255.0;
	_g = double(LoadAttributeSmall(you, "g", (int)(_g*255.0))) / 255.0;
	_b = double(LoadAttributeSmall(you, "b", (int)(_b*255.0))) / 255.0;
}

RGBColor::~RGBColor() {
}

namespace tj {
	namespace shared {
		std::wostream& operator<<(std::wostream& strm, const RGBColor& col) {
			strm.fill(L'0');
			strm << std::hex << std::setw(2) << std::uppercase << int(col.GetR()) <<
					std::hex << std::setw(2) << std::uppercase << int(col.GetG()) <<
					std::hex << std::setw(2) << std::uppercase << int(col.GetB());

			return strm;
		}

		std::ostream& operator<<(std::ostream& strm, const RGBColor& col) {
			strm.fill(L'0');
			strm << std::hex << std::setw(2) << std::uppercase << int(col.GetR()) <<
					std::hex << std::setw(2) << std::uppercase << int(col.GetG()) <<
					std::hex << std::setw(2) << std::uppercase << int(col.GetB());

			return strm;
		}
	}
}
