#ifndef _TJNETWORK_H
#define _TJNETWORK_H

namespace tj {
	namespace shared {
		class EXPORTED Network {
			public:
				static std::string GetHostName();
				static std::string GetHostAddress();
		};
	}
}

#endif