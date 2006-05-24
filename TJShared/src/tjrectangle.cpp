#include "../include/tjshared.h"
using namespace tj::shared;

Rectangle::Rectangle(int x, int y, int w, int h) {
	_x = x;
	_y = y;
	_w = w;
	_h = h;
}

Rectangle::Rectangle(const RECT& r) {
	_x = r.left;
	_y = r.top;
	_h = r.bottom-r.top;
	_w = r.right-r.left;
}

Rectangle::Rectangle(const Gdiplus::RectF& r) {
	_x = (int)r.GetLeft();
	_y = (int)r.GetTop();
	_h = int(r.GetBottom())-_y;
	_w = int(r.GetRight())-_x;
}

Rectangle::~Rectangle() {
}

int Rectangle::GetX() const {
	return _x;
}

int Rectangle::GetY() const {
	return _y;
}

int Rectangle::GetWidth() const {
	return _w;
}

int Rectangle::GetHeight() const {
	return _h;
}

void Rectangle::SetX(int x) {
	_x = x;
}

void Rectangle::SetY(int y) {
	_y = y;
}

void Rectangle::SetWidth(int w) {
	_w = w;
}

void Rectangle::SetHeight(int h) {
	_h = h;
}

void Rectangle::Narrow(int x, int y, int w, int h) {
	_x += x;
	_w -= x;
	_y += y;
	_h -= y;
	_h -= h;
	_w -= w;
}

void Rectangle::Widen(int x, int y, int w, int h) {
	Narrow(-x, -y, -w, -h);
}