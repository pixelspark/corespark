#include "../include/tjcore.h"
#include <math.h>
using namespace tj::shared;

Vector::Vector(float fx, float fy, float fz): x(fx), y(fy), z(fz) {
}

Vector::Vector(const Vector& other) {
	x = other.x;
	y = other.y;
	z = other.z;
}

Vector::~Vector() {
}

String Vector::ToString() const {
	std::wostringstream wos;
	wos << L'[' << x << L',' << y << L',' << z << L']';
	return wos.str();
}

Vector Vector::Scale(float s) const {
	return Vector(x*s, y*s, z*s);
}

Vector Vector::Dot(const Vector& other) const {
	return Vector(x*other.x, y*other.y, z*other.z);
}

float Vector::GetLength() const {
	return sqrt(x*x + y*y + z*z);
}

void Vector::Save(TiXmlElement* parent) {
	SaveAttributeSmall(parent, "x", x);
	SaveAttributeSmall(parent, "y", y);
	SaveAttributeSmall(parent, "z", z);
}

void Vector::Load(TiXmlElement* you) {
	x = LoadAttributeSmall(you, "x", x);
	y = LoadAttributeSmall(you, "y", y);
	z = LoadAttributeSmall(you, "z", z);
}