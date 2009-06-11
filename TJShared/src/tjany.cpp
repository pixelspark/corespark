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

Any::Any(const String& s): _stringValue(s), _type(TypeString) {
}

Any::Any(ref<Object> object): _object(object), _type(TypeObject) {
}

Any::Any(strong<Tuple> tuple): _object(ref<Tuple>(tuple)), _type(TypeTuple) {
}

Any::Any(Type t): _type(t) {
	_intValue = 0;
}

Any::Any(Type t, const String& s): _type(t) {
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

ref<Object> Any::GetContainedObject() {
	return _object;
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
				return Any(_stringValue + o._stringValue);
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

/**
Conversion table for operator== (* means always false).

		N	I	D	B	S	O
Null	X	*	*	*	*	*
Int		*	X	*	*	*	*
Double	*	*	X	*	*	*
Bool	*	*	*	X	*	*
String	*	*	*	*	X	*
Object	*	*	*	*	*	X
**/
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
	else if(t==TypeTuple) {
		if(_type==TypeTuple) {
			if(_object) {
				return Any(strong<Tuple>(_object));
			}
			return Any(Any::TypeTuple);
		}
		else {
			ref<Tuple> tuple = GC::Hold(new Tuple(1));
			tuple->Set(0, *this);
			return Any(strong<Tuple>(tuple));
		}
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

		case TypeDouble:
			return _doubleValue;

		case TypeString:
			return StringTo<double>(_stringValue, 0.0);

		default:
			return std::numeric_limits<double>::quiet_NaN();
	}	
}

Any::operator float() const {
	return float(this->operator double());
}

Any::operator int() const {
	switch(_type) {
		case TypeBool:
			return _boolValue ? 1 : 0;

		case TypeDouble:
			return int(_doubleValue);

		case TypeInteger:
			return _intValue;

		case TypeString:
			return StringTo<int>(_stringValue, 0);

		default:
			return 0;
	}	
}

String Any::ToString() const {
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

		case TypeTuple:
			if(_object) {
				return ref<Tuple>(_object)->ToString();
			}
			else {
				return L"[]";
			}
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

	if(_type==TypeTuple && _object) {
		ref<Tuple> tuple = _object;
		for(unsigned int a=0;a<tuple->GetLength();a++) {
			SaveAttributeSmall<unsigned int>(you, "length", tuple->GetLength());

			TiXmlElement element("element");
			tuple->Get(a).Save(&element);
			you->InsertEndChild(element);
		}
	}
}

void Any::Load(TiXmlElement* you) {
	_type = (Type)LoadAttributeSmall<int>(you, "type", (int)_type);
	String value = LoadAttributeSmall(you, "value", ToString());

	if(_type==TypeTuple) {
		TiXmlElement* element = you->FirstChildElement("element");
		unsigned int length = LoadAttributeSmall<unsigned int>(element, "length", 0);
		ref<Tuple> tuple = GC::Hold(new Tuple(length));


		for(unsigned int a=0; a<length; a++) {
			if(element==0) {
				break;
			}
			element = element->NextSiblingElement("element");
			Any val;
			val.Load(element);
			tuple->Set(a, val);
		}
		_object = tuple;
	}
	else {
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
}

ref<Property> Any::CreateTypeProperty(const String& name, ref<Inspectable> holder, Type* type) {
	assert(type!=0);
	ref< GenericListProperty<Type> > pt = GC::Hold(new GenericListProperty<Type>(name, holder, type, *type));
	pt->AddOption(TL(type_null), TypeNull);
	pt->AddOption(TL(type_integer), TypeInteger);
	pt->AddOption(TL(type_double), TypeDouble);
	pt->AddOption(TL(type_string), TypeString);
	pt->AddOption(TL(type_bool), TypeBool);
	pt->AddOption(TL(type_object), TypeObject);
	pt->AddOption(TL(type_tuple), TypeTuple);
	return pt;
}

String Any::GetTypeName(Type t) {
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

		case TypeTuple:
			return TL(type_tuple);
	}

	return L"";
}

/** Tuple **/
Tuple::Tuple(unsigned int size) : _length(size) {
	_data = new Any[size];
	for(unsigned int a=0;a<_length;a++) {
		_data[a] = Any();
	}
}

Tuple::~Tuple() {
	delete[] _data;
}

Any& Tuple::Get(unsigned int i) {
	if(i>=_length) {
		Throw(L"Tuple index out of range!", ExceptionTypeError);
	}
	return _data[i];
}

const Any& Tuple::Get(unsigned int i) const {
	if(i>=_length) {
		Throw(L"Tuple index out of range!", ExceptionTypeError);
	}
	return _data[i];
}

unsigned int Tuple::GetLength() const {
	return _length;
}

String Tuple::ToString() const {
	std::wostringstream wos;
	wos << L'[';
	for(unsigned int a=0;a<_length;a++) {
		wos << _data[a].ToString();
		if(a<(_length-1)) {
			wos << L',' << L' ';
		}
	}
	wos << L']';
	return wos.str();
}

void Tuple::Set(unsigned int i, const Any& v) {
	if(i>=_length) {
		Throw(L"Tuple index out of range!", ExceptionTypeError);
	}
	_data[i] = v;
}