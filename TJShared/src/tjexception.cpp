#include "../include/tjshared.h"
using namespace tj::shared;

String Exception::ToString() const {
	std::wostringstream os;
	os << _message;
	os << " (" << _file << ":" << _line << ")";

	return os.str();
}