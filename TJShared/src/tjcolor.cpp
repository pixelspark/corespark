#include "../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

const Pixels ColorPopupWnd::KWheelMargin = 5;

/** Black   = minimum(1-Red,1-Green,1-Blue)
    Cyan    = (1-Red-Black)/(1-Black)
    Magenta = (1-Green-Black)/(1-Black)
    Yellow  = (1-Blue-Black)/(1-Black) */

CMYKColor ColorSpaces::RGBToCMYK(double r, double g, double b) {
	CMYKColor cmyk(0.0, 0.0, 0.0, 0.0);

	cmyk._k = min(1.0-r, min(1.0-g, 1.0-b));
	if(cmyk._k<1.0) {
		cmyk._c = (1.0-r-cmyk._k)/(1.0-cmyk._k);
		cmyk._m = (1.0-g-cmyk._k)/(1.0-cmyk._k);
		cmyk._y = (1.0-b-cmyk._k)/(1.0-cmyk._k);
	}

	return cmyk;
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

	double min = min(min(r, g), b);
	double max = max(max(r, g), b);
	double v = max;
	double delta = max - min;

	if(max == 0.0 || delta == 0.0) {
		// R, G, and B must be 0, or all the same.
		// In this case, S is 0, and H is undefined.
		// Using H = 0 is as good as any...
		s = 0.0;
		h = 0.0;
	} 
	else {
		s = delta / max;
		if(r == max) {
			// Between Yellow and Magenta
			h = (g - b) / delta;
		} 
		else if(g == max) {
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
Color ColorSpaces::HSVToRGB(double h, double s, double v)  {
	double r,g,b;
	r = g = b = 0.0;

	// Scale Hue to be between 0 and 360. Saturation
	// and value scale to be between 0 and 1.
	h = fmod(h*360.0, 360.0);

	if(s == 0.0) {
		// Gray shade
		return Color(int(v*255.0f),int(v*255.0f),int(v*255.0f));
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

		return Color(BYTE(r*255.0), BYTE(g*255.0), BYTE(b*255.0));
	}
}


/* RGBColor */
void RGBColor::Save(TiXmlElement* parent) {
	TiXmlElement color("color");
	SaveAttributeSmall(&color,"r", (int)r);
	SaveAttributeSmall(&color, "g", (int)g);
	SaveAttributeSmall(&color, "b", (int)b);
	parent->InsertEndChild(color);
}

void RGBColor::Load(TiXmlElement* you) {
	r = LoadAttributeSmall(you, "r", (int)r);
	g = LoadAttributeSmall(you, "g", (int)g);
	b = LoadAttributeSmall(you, "b", (int)b);
}

RGBColor::operator Gdiplus::Color() {
	return Gdiplus::Color(r,g,b);
}

ColorWnd::ColorWnd(unsigned char r, unsigned char g, unsigned char b): ChildWnd(L"Color") {
	_r = r;
	_g = g;
	_b = b;
}

ColorWnd::~ColorWnd() {
}