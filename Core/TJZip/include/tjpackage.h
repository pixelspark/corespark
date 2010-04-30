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
 
 #ifndef _TJPACKAGE_H
#define _TJPACKAGE_H

namespace tj {
	namespace zip {
		class TJZIP_EXPORTED Package: public virtual tj::shared::Object {
			public:
				Package(const std::wstring& file, const char* passsword=0);
				virtual ~Package();
				virtual void Add(const std::wstring& filename, const std::wstring& realfile);
				virtual void AddData(const std::wstring& filename, const std::wstring& data);
			protected:
				HZIP _zip;
		};
	
	}
}

#endif