#include "../include/tjshared.h"

#include <sstream>

std::wstring Exception::ToString() {
	std::wostringstream os;
	os << _message;
	os << " (" << _file << ":" << _line << ")";

	return os.str();
}