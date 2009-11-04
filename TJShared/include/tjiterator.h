#ifndef _TJITERATOR_H
#define _TJITERATOR_H

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