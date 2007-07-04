#include "../include/tjshared.h"
#include "shlwapi.h"
using namespace tj::shared;

std::wstring File::GetDirectory(const std::wstring& pathToFile) {
	wchar_t* buf = _wcsdup(pathToFile.c_str());
	PathRemoveFileSpec(buf);
	std::wstring dir = buf;
	delete[] buf;
	return dir;
}

bool File::Exists(const std::wstring& st) {
	return GetFileAttributes(st.c_str())!=INVALID_FILE_ATTRIBUTES;
}