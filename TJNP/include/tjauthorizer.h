#ifndef _TJ_NP_AUTHORIZER_H
#define _TJ_NP_AUTHORIZER_H

#include "internal/tjnp.h"

namespace tj {
	namespace np {
		typedef unsigned int SecurityToken;

		/** The Authorizer class holds a private key and calculates 'security tokens' for objects, which can later
		be checked. This is used for resource authentication. Suppose client A asks for a resource called 'file.txt'. If
		the server has this file, it will send client A a URL to download this file. However, to prevent malicious clients
		to be able to download any file they want, it adds a security token to the URL. This token is basically a hash
		of a private key and the resource requested. If this hash is later checked, we can know for sure that the master
		originally sent a URL and the client is allowed to access this file. **/
		class NP_EXPORTED Authorizer {
			public:	
				Authorizer();
				Authorizer(unsigned int privateKey);
				virtual ~Authorizer();
				SecurityToken CreateToken(const tj::shared::String& object);
				bool CheckToken(const tj::shared::String& object, SecurityToken token);

			protected:
				unsigned int _privateKey;
		};
	}
}

#endif