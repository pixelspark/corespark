#include "../include/tjcore.h"
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <shlwapi.h>
#include <atlbase.h>

using namespace tj::shared;

std::wstring File::GetDirectory(const std::wstring& pathToFile) {
	wchar_t* buf = _wcsdup(pathToFile.c_str());
	PathRemoveFileSpec(buf);
	std::wstring dir = buf;
	delete[] buf;
	return dir;
}

std::wstring File::GetFileName(const std::wstring& pathToFile) {
	return std::wstring(PathFindFileName(pathToFile.c_str()));
}

std::wstring File::GetExtension(const std::wstring& pathToFile) {
	return std::wstring(PathFindExtension(pathToFile.c_str()));
}

bool File::Exists(const std::wstring& st) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	return GetFileAttributes(st.c_str())!=INVALID_FILE_ATTRIBUTES;
}

bool File::Move(const std::wstring& from, const std::wstring& to, bool silent) {
	ZoneEntry ze(Zones::LocalFileAdministrationZone);

	// Because SHCreateItemFromParsingName is not available under XP, find it dynamically
	typedef HRESULT (WINAPI* fnSHCreateItemFromParsingName)(PCWSTR, IBindCtx*, REFIID, void**);
	fnSHCreateItemFromParsingName pfnSHCreateItemFromParsingName = NULL;
	HMODULE shellLib = GetModuleHandle(L"shell32");
	if(shellLib) {
		// Attempt to get the address of SHCreateItemFromParsingName
		pfnSHCreateItemFromParsingName = (fnSHCreateItemFromParsingName)GetProcAddress(shellLib, "SHCreateItemFromParsingName");
		if(pfnSHCreateItemFromParsingName!=0) {
			/* IFileOperation has replaced SHFileOperation under Vista. Since some file moves (e.g. the TJShow license file
			move to ProgramData) requires 'elevation' (UAC), we need the IFileOperation to present it. */
			CComPtr<IFileOperation> op;
			if(SUCCEEDED(CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_IFileOperation, (void**)&op))) {
				op->SetOperationFlags(FOFX_SHOWELEVATIONPROMPT|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|(silent ? (FOF_NO_UI|FOF_SILENT) : 0));
				
				// Create source/dest shell items
				CComPtr<IShellItem> fromItem;
				CComPtr<IShellItem> toItem;

				std::wstring toFolder = File::GetDirectory(to);
				std::wstring toFileName = File::GetFileName(to);

				if(SUCCEEDED(pfnSHCreateItemFromParsingName(toFolder.c_str(), NULL, IID_PPV_ARGS(&toItem)))) {
					if(SUCCEEDED(pfnSHCreateItemFromParsingName(from.c_str(), NULL, IID_PPV_ARGS(&fromItem)))) {
						if(SUCCEEDED(op->MoveItem(fromItem, toItem, toFileName.c_str(), NULL))) {
							return true;
						}
					}
				}
			}
		}


	}

	// Fallback to SHFileOperation
	Log::Write(L"TJShared/File", L"Could not move file using IFileOperation; falling back to SHFileOperation");
	SHFILEOPSTRUCT shop;
	shop.pFrom = from.c_str();
	shop.pTo = to.c_str();
	shop.wFunc = FO_MOVE;
	shop.hwnd = NULL;
	shop.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|(silent ? FOF_SILENT : 0);
	shop.fAnyOperationsAborted = 0;
	shop.hNameMappings = 0;
	shop.lpszProgressTitle = 0L;

	return SHFileOperation(&shop) == 0;
}

bool File::Copy(const std::wstring& from, const std::wstring& to, bool silent) {
	ZoneEntry ze(Zones::LocalFileAdministrationZone);

	// TODO: this should try to use IFileOperation first too
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
