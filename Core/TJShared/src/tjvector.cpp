/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #include "../include/tjvector.h"
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