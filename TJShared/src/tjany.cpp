#include "../include/tjcore.h"
#include "../include/properties/tjproperties.h"
#include <limits>
using namespace tj::shared;

/* Any */
Any::Any(): _type(TypeNull) {
}

Any::Any(double d): _doubleValue(d), _type(TypeDouble) {
}

Any::Any(bool b): _boolValue(b), _type(TypeBool) {
}

Any::Any(int i): _intValue(i), _type(TypeInteger) {
}

Any::Any(const std::wstring& s): _stringValue(s), _type(TypeString) {
}

Any::Any(ref<Object> object): _object(object), _type(TypeObject) {
}

Any::Any(Type t): _type(t) {
	_intValue = 0;
}

Any::Any(Type t, const std::wstring& s): _type(t) {
	switch(t) {
		case TypeString:
			_stringValue = s;
			break;

		case TypeDouble:
			_doubleValue = StringTo<double>(s,0.0);
			break;

		case TypeBool:
			_boolValue = StringTo<bool>(s,false);
			break;

		case TypeInteger:
			_intValue = StringTo<int>(s,0);
			break;
	}
}

Any::~Any() {
}

/**
Conversion table for operator+ (* means null).

		N	I	D	B	S	O
Null	*	*	*	*	*	*
Int		*	i	d	*	s	*
Double	*	d	d	*	s	*
Bool	*	*	*	*	*	*
String	*	s	s	*	s	*
Object	*	*	*	*	*	*
**/
Any Any::operator+(const Any& o) const {
	switch(_type) {
		case TypeInteger:
			if(o._type==TypeInteger) {
				return Any(_intValue + o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(_intValue + o._doubleValue);
			}
			else if(o._type==TypeString) {
				return Any(Stringify(_intValue) + o._stringValue);
			}
			break;

		case TypeDouble:
			if(o._type==TypeInteger) {
				return Any(_doubleValue + o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(_doubleValue + o._doubleValue);
			}
			else if(o._type==TypeString) {
				return Any(Stringify(_doubleValue) + o._stringValue);
			}
			break;

		case TypeString:
			if(o._type==TypeInteger) {
				return Any(_stringValue + Stringify(o._intValue));
			}
			else if(o._type==TypeDouble) {
				return Any(_stringValue + Stringify(o._doubleValue));
			}
			else if(o._type==TypeString) {
				return Any(_stringValue + Stringify(o._stringValue));
			}
			break;
	}

	return Any();
}

/**
Conversion table for operator- (* means null).

		N	I	D	B	S	O
Null	*	*	*	*	*	*
Int		*	i	d	*	*	*
Double	*	d	d	*	*	*
Bool	*	*	*	*	*	*
String	*	*	*	*	*	*
Object	*	*	*	*	*	*
**/
Any Any::operator-(const Any& o) const {
	switch(_type) {
		case TypeInteger:
			if(o._type==TypeInteger) {
				return Any(_intValue - o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(_intValue - o._doubleValue);
			}
			break;

		case TypeDouble:
			if(o._type==TypeInteger) {
				return Any(_doubleValue - o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(_doubleValue - o._doubleValue);
			}
			break;
	}

	return Any();
}

/**
Conversion table for operator* (* means null).

		N	I	D	B	S	O
Null	*	*	*	*	*	*
Int		*	i	d	*	*	*
Double	*	d	d	*	*	*
Bool	*	*	*	*	*	*
String	*	*	*	*	*	*
Object	*	*	*	*	*	*
**/
Any Any::operator*(const Any& o) const {
	switch(_type) {
		case TypeInteger:
			if(o._type==TypeInteger) {
				return Any(_intValue * o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(_intValue * o._doubleValue);
			}
			break;

		case TypeDouble:
			if(o._type==TypeInteger) {
				return Any(_doubleValue * o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(_doubleValue * o._doubleValue);
			}
			break;
	}

	return Any();
}

/**
Conversion table for operator* (* means null).

		N	I	D	B	S	O
Null	*	*	*	*	*	*
Int		*	i	d	*	*	*
Double	*	d	d	*	*	*
Bool	*	*	*	*	*	*
String	*	*	*	*	*	*
Object	*	*	*	*	*	*
**/
Any Any::operator/(const Any& o) const {
	switch(_type) {
		case TypeInteger:
			if(o._type==TypeInteger) {
				return Any(_intValue / o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(_intValue / o._doubleValue);
			}
			break;

		case TypeDouble:
			if(o._type==TypeInteger) {
				return Any(_doubleValue / o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(_doubleValue / o._doubleValue);
			}
			break;
	}

	return Any();
}

/**
Conversion table for operator* (* means null).

		N	I	D	B	S	O
Null	*	*	*	*	*	*
Int		*	i	d	*	*	*
Double	*	d	d	*	*	*
Bool	*	*	*	*	*	*
String	*	*	*	*	*	*
Object	*	*	*	*	*	*
**/
Any Any::operator%(const Any& o) const {
	switch(_type) {
		case TypeInteger:
			if(o._type==TypeInteger) {
				return Any(_intValue % o._intValue);
			}
			else if(o._type==TypeDouble) {
				return Any(fmod((double)_intValue,o._doubleValue));
			}
			break;

		case TypeDouble:
			if(o._type==TypeInteger) {
				return Any(fmod(_doubleValue,(double)o._intValue));
			}
			else if(o._type==TypeDouble) {
				return Any(fmod(_doubleValue,o._doubleValue));
			}
			break;
	}

	return Any();
}

/**
Conversion table for operator* (* means null).

		N	I	D	B	S	O
Null	*	*	*	*	*	*
Int		*	X	X	*	*	*
Double	*	X	X	*	*	*
Bool	*	*	*	*	*	*
String	*	*	*	*	*	*
Object	*	*	*	*	*	*
**/
bool Any::operator>(const Any& o) const {
	switch(_type) {
		case TypeInteger:
			if(o._type==TypeInteger) {
				return _intValue > o._intValue;
			}
			else if(o._type==TypeDouble) {
				return _intValue > o._doubleValue;
			}
			break;

		case TypeDouble:
			if(o._type==TypeInteger) {
				return _doubleValue > o._intValue;
			}
			else if(o._type==TypeDouble) {
				return _doubleValue > o._doubleValue;
			}
			break;
	}
	return false;
}

bool Any::operator<(const Any& o) const {
	switch(_type) {
		case TypeInteger:
			if(o._type==TypeInteger) {
				return _intValue < o._intValue;
			}
			else if(o._type==TypeDouble) {
				return _intValue < o._doubleValue;
			}
			break;

		case TypeDouble:
			if(o._type==TypeInteger) {
				return _doubleValue < o._intValue;
			}
			else if(o._type==TypeDouble) {
				return _doubleValue < o._doubleValue;
			}
			break;
	}
	return false;
}

bool Any::operator==(const Any& o) const {
	if(_type!=o._type) {
		return false;
	}
	else {
		switch(_type) {
			case TypeNull:
				return true;

			case TypeBool:
				return _boolValue == o._boolValue;

			case TypeInteger:
				return _intValue == o._intValue;

			case TypeDouble:
				return _doubleValue == o._doubleValue;
				
			case TypeString:
				return _stringValue == o._stringValue;

			case TypeObject:
				return _object == o._object;
		}
	}
	return false;
}

bool Any::operator||(const Any& o) const {
	if(_type==TypeBool && o._type==TypeBool) {
		return _boolValue || o._boolValue;
	}
	return false;
}

bool Any::operator&&(const Any& o) const {
	if(_type==TypeBool && o._type==TypeBool) {
		return _boolValue && o._boolValue;
	}
	return false;
}

bool Any::operator!=(const Any& o) const {
	return !this->operator==(o);
}

Any Any::Force(Any::Type t) const {
	if(t==_type) {
		return *this;
	}
	else if(t==TypeNull) {
		return Any();
	}
	else if(t==TypeInteger) {
		return Any(this->operator int());
	}
	else if(t==TypeDouble) {
		return Any(this->operator double());
	}
	else if(t==TypeBool) {
		return Any(this->operator bool());
	}
	else if(t==TypeString) {
		return Any(ToString());
	}
	else if(t==TypeObject) {
		return Any(ref<Object>(0));
	}

	return Any(); // typically everything with Object
}

Any::operator bool() const {
	switch(_type) {
		case TypeDouble:
			return _doubleValue != 0.0;

		case TypeInteger:
			return _intValue == 1;

		case TypeString:
			return _stringValue == L"true";

		case TypeBool:
			return _boolValue;

		case TypeObject:
			return _object;

		case TypeNull:
		default:
			return false;
	}	
}

Any::operator double() const {
	switch(_type) {
		case TypeBool:
			return _boolValue ? 1.0 : 0.0;

		case TypeInteger:
			return double(_intValue);

		case TypeString:
			return StringTo<double>(_stringValue, 0.0);

		default:
			return std::numeric_limits<double>::quiet_NaN();
	}	
}

Any::operator int() const {
	switch(_type) {
		case TypeBool:
			return _boolValue ? 1 : 0;

		case TypeDouble:
			return int(_doubleValue);

		case TypeString:
			return StringTo<int>(_stringValue, 0);

		default:
			return 0;
	}	
}

std::wstring Any::ToString() const {
	switch(_type) {
		case TypeNull:
			return L"null";

		case TypeInteger:
			return Stringify(_intValue);

		case TypeDouble:
			return Stringify(_doubleValue);

		case TypeBool:
			return Stringify(_boolValue);

		case TypeString:
			return _stringValue;

		case TypeObject:
			return L"[object]";
	}

	return L"[undefined]";
}

Any Any::operator-() const {
	switch(_type) {
		case TypeInteger:
			return Any(-_intValue);

		case TypeDouble:
			return Any(-_doubleValue);
	}

	return Any();
}

Any Any::operator!() const {
	if(_type==TypeBool) {
		return Any(!_boolValue);
	}

	return Any();
}

Any Any::Abs() const {
	switch(_type) {
		case TypeInteger:
			return Any(abs(_intValue));

		case TypeDouble:
			return Any(abs(_doubleValue));
	}

	return *this;
}

Any::Type Any::GetType() const {
	return _type;
}

void Any::Save(TiXmlElement* you) {
	SaveAttributeSmall<int>(you, "type", (int)_type);
	SaveAttributeSmall(you, "value", ToString());
}

void Any::Load(TiXmlElement* you) {
	_type = (Type)LoadAttributeSmall<int>(you, "type", (int)_type);
	std::wstring value = LoadAttributeSmall(you, "value", ToString());

	switch(_type) {
		case TypeInteger:
			_intValue = StringTo<int>(value, 0);
			break;

		case TypeDouble:
			_doubleValue = StringTo<double>(value, 0);
			break;

		case TypeBool:
			_boolValue = StringTo<bool>(value,0);
			break;

		case TypeString:
			_stringValue = value;
			break;

		case TypeObject:
			break;
	}
}

ref<Property> Any::CreateTypeProperty(const std::wstring& name, Type* type) {
	assert(type!=0);
	ref< GenericListProperty<Type> > pt = GC::Hold(new GenericListProperty<Type>(name, type, 0, *type));
	pt->AddOption(TL(type_null), TypeNull);
	pt->AddOption(TL(type_integer), TypeInteger);
	pt->AddOption(TL(type_double), TypeDouble);
	pt->AddOption(TL(type_string), TypeString);
	pt->AddOption(TL(type_bool), TypeBool);
	pt->AddOption(TL(type_object), TypeObject);
	return pt;
}

std::wstring Any::GetTypeName(Type t) {
	switch(t) {
		case TypeNull:
			return TL(type_null);

		case TypeInteger:
			return TL(type_integer);
			
		case TypeDouble:
			return TL(type_double);

		case TypeString:
			return TL(type_string);

		case TypeBool:
			return TL(type_bool);

		case TypeObject:
			return TL(type_object);
	}

	return L"";
}