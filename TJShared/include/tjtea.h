#ifndef _TJTEA_H
#define _TJTEA_H

namespace tj {
	namespace shared {
		class EXPORTED TEAEncrypter: public virtual Object {
			public:
				TEAEncrypter();
				virtual ~TEAEncrypter();
				std::string Encrypt(const std::string& src, const std::string& key);
				std::string Decrypt(const std::string& src, const std::string& key);
		};
	}
}

#endif