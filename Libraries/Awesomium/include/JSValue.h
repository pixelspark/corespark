/*
	This file is part of Awesomium, a library that helps facilitate
	the rendering of Web Content in an off-screen context.

	Copyright (C) 2008 Adam J. Simmons

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __JSVALUE_H__
#define __JSVALUE_H__

#include <string>
#include <vector>
#include "PlatformUtils.h"

class CppVariant;
class ClientObject;
class NamedCallback;

namespace Awesomium 
{

/**
* JSValue is a class that represents a Javascript value.
*/
class _OSMExport JSValue
{
public:
	JSValue();
	JSValue(bool value);
	JSValue(int value);
	JSValue(double value);
	JSValue(const char* value);
	JSValue(const std::string& value);
	JSValue(const JSValue& original);

	JSValue& operator=(const JSValue& original);

	~JSValue();

	bool isBool() const;
	bool isInteger() const;
	bool isDouble() const;
	bool isNumber() const;
	bool isString() const;
	bool isVoid() const;
	bool isNull() const;
	bool isEmpty() const;
	bool isObject() const;

	std::string toString() const;
	int toInteger() const;
	double toDouble() const;
	bool toBoolean() const;

protected:
	JSValue(CppVariant* value);
	JSValue(const CppVariant* value);
	CppVariant* value;
	friend class ClientObject;
	friend class NamedCallback;
};

typedef std::vector<JSValue> JSArguments;

}

#endif