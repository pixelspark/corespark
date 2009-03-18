#include "../include/tjcore.h"
using namespace tj::shared;

Code::Code(const char* code, unsigned int size) {
	_code = new char[size*sizeof(char)];
	memcpy(_code,code,size*sizeof(char));
	_size = size;
}

Code::~Code() {
	delete[] _code;
	_size = 0;
}

unsigned int Code::GetSize() {
	return _size;
}

CodeWriter::CodeWriter(unsigned int initSize) {
	_buffer = new char[initSize];
	_size = initSize;
	_pos = 0;
}

unsigned int CodeWriter::GetSize() {
	return _pos;
}

unsigned int CodeWriter::GetCapacity() {
	return _size;
}

CodeWriter::~CodeWriter() {
	if(_buffer!=0) {
		delete[] _buffer;
	}
}

void CodeWriter::Grow(unsigned int size) {
	if(_buffer==0) {
		Throw(L"CodeWriter grown after buffer has been taken over!", ExceptionTypeSevere);
	}

	if(_pos+size>_size) {
		unsigned int newSize = max(_size*2, _size+size);
		char* newBuffer = new char[newSize];
		for(unsigned int a=0;a<_pos;a++) {
			newBuffer[a] = _buffer[a];
		}
		_size = newSize;
		delete[] _buffer;
		_buffer = newBuffer;
	}
}

template<> tj::shared::Vector Code::Get(unsigned int& position) {
	Vector v(0.0f, 0.0f, 0.0f);
	v.x = Get<float>(position);
	v.y = Get<float>(position);
	v.z = Get<float>(position);
	return v;
}

template<> CodeWriter& CodeWriter::Add(const String& x) {
	if(_buffer==0) {
		Throw(L"CodeWriter written to after buffer has been taken over!", ExceptionTypeSevere);
	}

	Grow((unsigned int)((x.length()*sizeof(wchar_t))+sizeof(unsigned int)));

	Add<unsigned int>((unsigned int)x.length());
	String::const_iterator it = x.begin();
	while(it!=x.end()) {
		wchar_t c = *it;
		Add<wchar_t>(c);
		++it;
	}
	return *this;
}

template<> String Code::Get(unsigned int& position) {
	unsigned int length = Get<unsigned int>(position);
	std::wostringstream os;
	for(unsigned int a=0;a<length;a++) {
		os << (Get<wchar_t>(position));
	}

	return os.str();
}