#include "../include/tjshared.h"
using namespace tj::shared;

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