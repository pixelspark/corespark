#ifndef _TJFILE_H
#define _TJFILE_H

namespace tj {
	namespace shared {
		class EXPORTED File {
			public:
				static std::wstring GetDirectory(const std::wstring& pathToFile);
				static std::wstring GetFileName(const std::wstring& pathToFile);
				static std::wstring GetExtension(const std::wstring& pathToFile);

				static bool Exists(const std::wstring& path);
				static Bytes GetDirectorySize(const std::wstring& dirPath);
				
				static bool Move(const std::wstring& from, const std::wstring& to, bool silent = true);
				static bool Copy(const std::wstring& from, const std::wstring& to, bool silent = true);

				static const wchar_t PathSeparator = L'\\';
		};
	}
}
#endif