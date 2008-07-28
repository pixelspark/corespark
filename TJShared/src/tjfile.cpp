#include "../include/tjcore.h"
#include <shlwapi.h>
#include <shellapi.h>
using namespace tj::shared;

std::wstring File::GetDirectory(const std::wstring& pathToFile) {
	wchar_t* buf = _wcsdup(pathToFile.c_str());
	PathRemoveFileSpec(buf);
	std::wstring dir = buf;
	delete[] buf;
	return dir;
}

std::wstring File::GetExtension(const std::wstring& pathToFile) {
	return std::wstring(PathFindExtension(pathToFile.c_str()));
}

bool File::Exists(const std::wstring& st) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	return GetFileAttributes(st.c_str())!=INVALID_FILE_ATTRIBUTES;
}

bool File::Move(const std::wstring& from, const std::wstring& to, bool silent) {
	SHFILEOPSTRUCT op;
	op.pFrom = from.c_str();
	op.pTo = to.c_str();
	op.wFunc = FO_MOVE;
	op.hwnd = NULL;
	op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|(silent ? FOF_SILENT : 0);
	op.fAnyOperationsAborted = 0;
	op.hNameMappings = 0;
	op.lpszProgressTitle = 0L;

	return SHFileOperation(&op) == 0;
}

bool File::Copy(const std::wstring& from, const std::wstring& to, bool silent) {
	SHFILEOPSTRUCT op;
	op.pFrom = from.c_str();
	op.pTo = to.c_str();
	op.wFunc = FO_COPY;
	op.hwnd = NULL;
	op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|(silent ? FOF_SILENT : 0);
	op.fAnyOperationsAborted = 0;
	op.hNameMappings = 0;
	op.lpszProgressTitle = 0L;

	return SHFileOperation(&op) == 0;
}

Bytes File::GetDirectorySize(const std::wstring& dirPath) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	WIN32_FIND_DATA fd;
	std::wstring searchPath = dirPath+L"*";
	HANDLE search = FindFirstFile(searchPath.c_str(), &fd);

	if(search==INVALID_HANDLE_VALUE) {
		Log::Write(L"TJShared/File", L"Could not get directory size for '"+dirPath+L"'");
		return 0;
	}

	LARGE_INTEGER totalSize;
	totalSize.QuadPart = 0;

	do {
		if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0 && fd.cFileName[0]!=L'.') {
			// calculate directory size
			std::wstring subDirPath = dirPath + fd.cFileName + L"\\";
			Bytes dirSize = GetDirectorySize(subDirPath);
			totalSize.QuadPart += dirSize;
		}
		else {
			LARGE_INTEGER size;
			size.LowPart = fd.nFileSizeLow;
			size.HighPart = fd.nFileSizeHigh;
			totalSize.QuadPart += size.QuadPart;
		}
	} 
	while(FindNextFile(search, &fd));

	FindClose(search);
	return (Bytes)totalSize.QuadPart;
}
