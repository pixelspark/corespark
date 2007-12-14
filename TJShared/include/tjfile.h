#ifndef _TJFILE_H
#define _TJFILE_H

namespace tj {
	namespace shared {
		typedef long long Bytes; // This is equivalent to __int64 on MSVC++

		class EXPORTED File {
			public:
				static std::wstring GetDirectory(const std::wstring& pathToFile);
				static bool Exists(const std::wstring& path);
				static Bytes GetDirectorySize(const std::wstring& dirPath);
		};
	}
}
#endif