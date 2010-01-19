#ifndef _TJUPDATERFILEHASH_H
#define _TJUPDATERFILEHASH_H

namespace tj {
	namespace updater {
		bool MD5HashFile(const std::wstring& path, std::wstring& hash);
	}
}
#endif