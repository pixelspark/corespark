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
 
 #ifndef _TJHASH_H
#define _TJHASH_H

#include "tjsharedinternal.h"

namespace tj {
	namespace shared {
		// FNV hash
		class EXPORTED Hash: public virtual Object {
			public:
				Hash();
				virtual ~Hash();
				int Calculate(const String& data);
		};

		class EXPORTED Hasheable {
			public:
				virtual ~Hasheable();
				virtual int GetHash() const = 0;
		};

		// This is a MD5 hash
		class EXPORTED SecureHash: public virtual Object {
			public:
				SecureHash();
				virtual ~SecureHash();
				virtual void AddData(const void* data, size_t length);
				virtual std::string GetHashAsString();
				virtual void AddString(const wchar_t* data);
				virtual void AddFile(const String& path);

			protected:
				void* _data;
		};

		// TEA (Tiny Encryption Algorithm)
		// This CANNOT be used for hashing! Use (Secure)Hash for that instead
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