#include "../include/tjshared.h"

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
	delete[] _buffer;
}


template<> CodeWriter& CodeWriter::Add(std::wstring x) {
	Add((unsigned short)x.length());
	std::wstring::iterator it = x.begin();
	while(it!=x.end()) {
		wchar_t c = *it;
		Add(c);
		it++;
	}
	return *this;
}