#include "../include/tjcode.h"
#include "../include/tjutil.h"
#include "../include/tjvector.h"
#include "../include/tjserializable.h"
using namespace tj::shared;

Data::~Data() {
}

DataReader::DataReader(const char* code, Bytes size) {
	_code = new char[(size_t)(size*sizeof(char))];
	memcpy(_code,code,(size_t)(size*sizeof(char)));
	_size = size;
}

DataReader::~DataReader() {
	delete[] _code;
	_size = 0;
}

const char* DataReader::GetBuffer() const {
	return _code;
}

char* DataReader::TakeOverBuffer(bool clearMine) {
	if(clearMine) {
		char* buf = _code;
		_size = 0;
		_code = 0;
		return buf;
	}
	else {
		char* buffer = new char[(size_t)_size];
		memcpy(buffer, _code, (size_t)_size);
		return buffer;
	}
}

Bytes DataReader::GetSize() const {
	return _size;
}

/** DataWriter **/
DataWriter::DataWriter(Bytes initSize) {
	_buffer = new char[(size_t)initSize];
	_size = initSize;
	_pos = 0;
}

const char* DataWriter::GetBuffer() const {
	return _buffer;
}

char* DataWriter::TakeOverBuffer(bool clearMine) {
	char* buf = _buffer;
	if(clearMine) {
		_buffer = 0;
		_size = 0;
		_pos = 0;
	}
	else {
		_buffer = new char[(size_t)_size];
		memcpy(_buffer, buf, (size_t)_pos);
	}
	return buf;
}

Bytes DataWriter::GetSize() const {
	return _pos;
}

Bytes DataWriter::GetCapacity() const {
	return _size;
}

DataWriter::~DataWriter() {
	if(_buffer!=0) {
		delete[] _buffer;
	}
}

void DataWriter::Grow(Bytes size) {
	if(_buffer==0) {
		Throw(L"DataWriter grown after buffer has been taken over!", ExceptionTypeSevere);
	}

	if(_pos+size>_size) {
		unsigned int newSize = (size_t)(Util::Max(_size*2, _size+size));
		char* newBuffer = new char[newSize];
		for(unsigned int a=0;a<_pos;a++) {
			newBuffer[a] = _buffer[a];
		}
		_size = newSize;
		delete[] _buffer;
		_buffer = newBuffer;
	}
}

template<> tj::shared::Vector DataReader::Get(unsigned int& position) {
	Vector v(0.0f, 0.0f, 0.0f);
	v.x = Get<float>(position);
	v.y = Get<float>(position);
	v.z = Get<float>(position);
	return v;
}

template<> DataWriter& DataWriter::Add(const String& x) {
	if(_buffer==0) {
		Throw(L"DataWriter written to after buffer has been taken over!", ExceptionTypeSevere);
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

template<> String DataReader::Get(unsigned int& position) {
	unsigned int length = Get<unsigned int>(position);
	std::wostringstream os;
	for(unsigned int a=0;a<length;a++) {
		os << (Get<wchar_t>(position));
	}

	return os.str();
}
