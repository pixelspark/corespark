#include "../include/tjshared.h"
using namespace tj::shared;


std::wstring Exception::ToString() const {
	std::wostringstream os;
	os << _message;
	os << " (" << _file << ":" << _line << ")";

	return os.str();
}