#ifndef _TJCOLOR_H
#define _TJCOLOR_H

struct EXPORTED RGBColor: public Serializable {
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

#endif