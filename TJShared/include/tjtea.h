#ifndef _TJTEA_H
#define _TJTEA_H

#include <string>

class EXPORTED TEAEncrypter {
	public:
		TEAEncrypter();
		virtual ~TEAEncrypter();
		std::string Encrypt(std::string src, std::string key);
		std::string Decrypt(std::string src, std::string key);
};

#endif