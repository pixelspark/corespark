#include "../include/tjfile.h"
#include "../include/tjzone.h"
#include "../include/tjutil.h"

#ifdef TJ_OS_WIN
	#include <shellapi.h>
	#include <shlobj.h>
	#include <shobjidl.h>
	#include <shlguid.h>
	#include <shlwapi.h>
#endif

#ifdef TJ_OS_POSIX
	#include <libgen.h>
	#include <sys/stat.h>
	#include <stdio.h>
#endif

#ifdef TJ_OS_MAC
	#include <copyfile.h>
#endif

using namespace tj::shared;


wchar_t File::GetPathSeparator() {
	#ifdef TJ_OS_WIN
		return L'\\';
	#endif

	#ifdef TJ_OS_POSIX
		return L'/';
	#endif
}

String File::GetDirectory(const String& pathToFile) {
	#ifdef TJ_OS_POSIX
		std::string path = Mbs(pathToFile);
		std::string dirPath(dirname(const_cast<char*>(path.c_str())));
		return Wcs(dirPath);
	#endif
	
	#ifdef TJ_OS_WIN
		wchar_t* buf = _wcsdup(pathToFile.c_str());
		PathRemoveFileSpec(buf);
		String dir = buf;
		delete[] buf;
		return dir;
	#endif
}

String File::GetFileName(const String& pathToFile) {
	#ifdef TJ_OS_WIN
		return String(PathFindFileName(pathToFile.c_str()));
	#endif
	
	#ifdef TJ_OS_POSIX
		std::string path = Mbs(pathToFile);
		std::string fileName(basename(const_cast<char*>(path.c_str())));
		return Wcs(fileName);
	#endif
}

String File::GetExtension(const String& pathToFile) {
	#ifdef TJ_OS_POSIX
		String fileName = File::GetFileName(pathToFile);
		return std::wstring(fileName,0,fileName.find_last_of(L'.'));
	#endif
	
	#ifdef TJ_OS_WIN
		return String(PathFindExtension(pathToFile.c_str()));
	#endif
}

bool File::Exists(const String& st) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	
	#ifdef TJ_OS_WIN
		return GetFileAttributes(st.c_str())!=INVALID_FILE_ATTRIBUTES;
	#endif
	
	#ifdef TJ_OS_POSIX
		struct stat s;
		std::string mbsFile = Mbs(st);
		if(stat(mbsFile.c_str(), &s)==0) {
			return true;
		}
	
		return false;
	#endif
}

bool File::Move(const String& from, const String& to, bool silent) {
	ZoneEntry ze(Zones::LocalFileAdministrationZone);
	
	#ifdef TJ_OS_WIN
		// If the target directory doesn't exist, create it
		String dir = File::GetDirectory(to);
		SHCreateDirectory(NULL, dir.c_str());
		return MoveFile(from.c_str(), to.c_str())==TRUE;
	#endif
	
	#ifdef TJ_OS_POSIX
		std::string mbsFrom = Mbs(from);
		std::string mbsTo = Mbs(to);
		return (rename(mbsFrom.c_str(), mbsTo.c_str())==0);
	#endif
}

bool File::Copy(const String& from, const String& to, bool silent) {
	ZoneEntry ze(Zones::LocalFileAdministrationZone);

	#ifdef TJ_OS_WIN
		// TODO: this should try to use IFileOperation first too, since this replaces SHFileOperation from Windows Vista onwards
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
	#endif
	
	#ifdef TJ_OS_POSIX
		#ifdef TJ_OS_MAC
			std::string fromMbs = Mbs(from);
			std::string toMbs = Mbs(to);
	
			// Like SHFileOperation, copyfile on Mac will also work recursively when two directories are specified
			// But only if COPYFILE_RECURSIVE is specified (and that doesn't work somehow when building client apps)
			if(copyfile(fromMbs.c_str(), toMbs.c_str(), NULL, COPYFILE_ALL)!=0) {
				// TODO: Error occurred, check the return value of copyfile
				return false;
			}
			return true;
		#else
			#warning Not implemented on POSIX (File::Copy)
			return false;
		#endif
	#endif
}

void File::DeleteFiles(const String& dir, const String& pattern) {
	ZoneEntry ze(Zones::LocalFileAdministrationZone);

	#ifdef TJ_OS_WIN
		if(dir.size()>0) {
			String wc = dir + L"\\" + pattern;
			// Empty the cache
			SHFILEOPSTRUCT shop;
			shop.hwnd = 0L;
			shop.fAnyOperationsAborted = FALSE;
			shop.hNameMappings = NULL ;
			shop.lpszProgressTitle = 0L;
			shop.wFunc = FO_DELETE;
			shop.pFrom = wc.c_str();
			shop.pTo = 0;
			shop.fFlags = FOF_SILENT|FOF_NOCONFIRMATION;
			SHFileOperation(&shop);
		}
	#endif
	
	#ifdef TJ_OS_POSIX
		#warning Not implemented on POSIX (File::DeleteFiles with pattern)
	#endif
}

Bytes File::GetFileSize(const String& filePath) {
	ZoneEntry ze(Zones::LocalFileInfoZone);

	#ifdef TJ_OS_WIN
		HANDLE file = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		if(file!=INVALID_HANDLE_VALUE) {
			LARGE_INTEGER li;
			if(GetFileSizeEx(file, &li)==TRUE) {
				return (Bytes)li.QuadPart;
			}
			CloseHandle(file);
		}
		return -1;
	#endif
	
	#ifdef TJ_OS_POSIX
		std::string mbsPath = Mbs(filePath);
		struct stat st;
		if(stat(mbsPath.c_str(), &st)==0) {
			return st.st_size;
		}
		return -1;
	#endif
}

Bytes File::GetDirectorySize(const String& dirPath) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	
	#ifdef TJ_OS_WIN
		WIN32_FIND_DATA fd;
		String searchPath = dirPath+L"*";
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
				String subDirPath = dirPath + fd.cFileName + L"\\";
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
	#endif
	
	#ifdef TJ_OS_POSIX
		#warning Not implemented on POSIX (File::GetDirectorySize)
		return 0;
	#endif
}
