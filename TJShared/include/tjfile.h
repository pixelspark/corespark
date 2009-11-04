#ifndef _TJFILE_H
#define _TJFILE_H

#include "internal/tjpch.h"
#include "../include/tjutil.h"

namespace tj {
	namespace shared {
		class EXPORTED File {
			public:
				static String GetDirectory(const String& pathToFile);
				static String GetFileName(const String& pathToFile);
				static String GetExtension(const String& pathToFile);

				static bool Exists(const String& path);
				static Bytes GetDirectorySize(const String& dirPath);
				static Bytes GetFileSize(const String& filePath);
				static void DeleteFiles(const String& dir, const String& pattern);
				
				static bool Move(const String& from, const String& to, bool silent = true);
				static bool Copy(const String& from, const String& to, bool silent = true);

				static wchar_t GetPathSeparator();
		};
	}
}
#endif