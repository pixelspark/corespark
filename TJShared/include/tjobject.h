#ifndef _TJOBJECT_H
#define _TJOBJECT_H

class EXPORTED Object {
	friend class GC;

	public:
		template<class T> inline ref<T> This() {
			if(_resource!=0) {
				Resource<T>* tr = reinterpret_cast< Resource<T>* >(_resource);
				return tr->Reference();
			}
			return 0;
		}

	private:
		Resource<Object>* _resource;
};

#endif