#ifndef _TJUPDATERIDENTIFICATION_H
#define _TJUPDATERIDENTIFICATION_H

namespace tj {
	namespace updater {
		class Machine {
			public:
				static void GetUniqueIdentifier(std::wstring& mac);
		};
	}
}

#endif