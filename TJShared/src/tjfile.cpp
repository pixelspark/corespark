#include "../include/tjshared.h"
#include "shlwapi.h"
using namespace tj::shared;

std::wstring File::GetDirectory(std::wstring pathToFile) {
	wchar_t* buf = _wcsdup(pathToFile.c_str());
	PathRemoveFileSpec(buf);
	std::wstring dir = buf;
	delete[] buf;
	return dir;
}