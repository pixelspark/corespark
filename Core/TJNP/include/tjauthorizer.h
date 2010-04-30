/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_NP_AUTHORIZER_H
#define _TJ_NP_AUTHORIZER_H

#include "tjnpinternal.h"

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