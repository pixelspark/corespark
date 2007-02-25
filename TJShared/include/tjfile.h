#ifndef _TJFILE_H
#define _TJFILE_H

namespace tj {
	namespace shared {
		class EXPORTED File {
			public:
				static std::wstring GetDirectory(std::wstring pathToFile);
		};
	}
}
#endif