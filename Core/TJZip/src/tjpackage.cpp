#include "../include/tjzip.h"
using namespace tj::zip;
using namespace tj::shared;

#include "zip.h"
#include "unzip.h"

Copyright KCRInfoZip(L"TJZip", L"Info-zip", L"©1990-1999 Info-ZIP. All rights reserved.");

Package::Package(const std::wstring& file, const char* password) {
	if(GetFileAttributes(file.c_str())!=INVALID_FILE_ATTRIBUTES) {
		_zip = OpenZip(file.c_str(), password);
	}
	else {
		_zip = CreateZip(file.c_str(),password);
	}
}

Package::~Package() {
	CloseZip(_zip);
}

void Package::Add(const std::wstring& file, const std::wstring& real) {
	ZipAdd(_zip, file.c_str(), real.c_str());
}

void Package::AddData(const std::wstring& file, const std::wstring& data) {
	ZipAdd(_zip, file.c_str(),(void*)data.c_str(), (unsigned int)(data.length()*sizeof(wchar_t)));
}