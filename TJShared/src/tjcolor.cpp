#include "../include/tjshared.h"
using namespace tj::shared;

void RGBColor::Save(TiXmlElement* parent) {
	TiXmlElement color("color");
	SaveAttribute(&color,"r", (int)r);
	SaveAttribute(&color, "g", (int)g);
	SaveAttribute(&color, "b", (int)b);
	parent->InsertEndChild(color);
}

void RGBColor::Load(TiXmlElement* you) {
	r = LoadAttribute(you, "r", (int)r);
	g = LoadAttribute(you, "g", (int)g);
	b = LoadAttribute(you, "b", (int)b);
}

RGBColor::operator Gdiplus::Color() {
	return Gdiplus::Color(r,g,b);
}

ColorWnd::ColorWnd(HWND parent, unsigned char r, unsigned char g, unsigned char b): ChildWnd(L"Color", parent) {
	_r = r;
	_g = g;
	_b = b;
}

ColorWnd::~ColorWnd() {
}