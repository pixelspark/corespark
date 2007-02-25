#ifndef _TJTEA_H
#define _TJTEA_H

namespace tj {
	namespace shared {
		class EXPORTED TEAEncrypter: public virtual Object {
			public:
				TEAEncrypter();
				virtual ~TEAEncrypter();
				std::string Encrypt(std::string src, std::string key);
				std::string Decrypt(std::string src, std::string key);
		};
	}
}

#endif