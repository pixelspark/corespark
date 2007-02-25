#include "../include/tjshared.h"
#include <math.h>
using namespace tj::shared;
using namespace Gdiplus;

Vector::Vector(float fx, float fy, float fz): x(fx), y(fy), z(fz) {
}

Vector::Vector(const Vector& other) {
	x = other.x;
	y = other.y;
	z = other.z;
}

Vector::~Vector() {
}

std::wstring Vector::ToString() const {
	std::wostringstream wos;
	wos << L'[' << x << L',' << y << L',' << z << L']';
	return wos.str();
}

Vector Vector::Dot(const Vector& other) const {
	return Vector(x*other.x, y*other.y, z*other.z);
}

float Vector::GetLength() const {
	return sqrt(x*x + y*y + z*z);
}

void Vector::Save(TiXmlElement* parent) {
	SaveAttribute(parent, "x", x);
	SaveAttribute(parent, "y", y);
	SaveAttribute(parent, "z", z);
}

void Vector::Load(TiXmlElement* you) {
	x = LoadAttribute(you, "x", x);
	y = LoadAttribute(you, "y", y);
	z = LoadAttribute(you, "z", z);
}