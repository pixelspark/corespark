#ifndef _TJOBJECT_H
#define _TJOBJECT_H

namespace tj {
	namespace shared {
		class EXPORTED Object {
			friend class GC;

			public:
				template<class T> inline ref<T> This() {
					if(_resource!=0) {
						tj::shared::intern::Resource<T>* tr = reinterpret_cast< tj::shared::intern::Resource<T>* >(_resource);
						return tr->Reference();
					}
					return 0;
				}

			private:
				tj::shared::intern::Resource<Object>* _resource;
		};
	}
}

#endif