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
 
 #ifndef _TJITERATOR_H
#define _TJITERATOR_H

#include "tjsharedinternal.h"
#include <vector>

namespace tj {
	namespace shared {
		template<typename T> class Iterator {
			public:
				virtual ~Iterator() {
				}

				virtual void Next() = 0;
				virtual bool IsValid() = 0;
				virtual T Get() = 0;

				operator bool() {
					return IsValid();
				}
		};

		template<typename T> class VectorIterator: public Iterator<T> {
			public:
				VectorIterator(std::vector<T>& vector) {
					_it = vector.begin();
					_end = vector.end();
				}

				virtual ~VectorIterator() {
				}

				virtual void Next() {
					if(_it!=_end) {
						++_it;
					}
				}

				virtual bool IsValid() {
					return _it!=_end;
				}

				virtual T Get() {
					return *_it;
				}

			protected:
				typename std::vector<T>::iterator _it;
				typename std::vector<T>::iterator _end;
		};
	}
}

#endif