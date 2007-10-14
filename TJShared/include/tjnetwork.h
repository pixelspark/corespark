#ifndef _TJNETWORK_H
#define _TJNETWORK_H

namespace tj {
	namespace shared {
		class EXPORTED Networking {
			public:
				static std::string GetHostName();
				static std::string GetHostAddress();
		};
	}
}

#endif