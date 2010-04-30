/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJFILE_H
#define _TJFILE_H

#include "tjsharedinternal.h"
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
				static bool CreateDirectoryAtPath(const String& path, bool recursive);
				
				static bool Move(const String& from, const String& to, bool silent = true);
				static bool Copy(const String& from, const String& to, bool silent = true);

				static wchar_t GetPathSeparator();
		};
	}
}
#endif