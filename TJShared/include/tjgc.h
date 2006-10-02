#ifndef _TJGC_H
#define _TJGC_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED GC {
	public:
		static void IncrementLive(size_t size=0);
		static void DecrementLive(size_t size=0);
		static long GetLiveCount();
		static long GetSize();
		template<typename T> static ref< T > Hold(T* x);

	protected:
		static inline void SetObjectPointer(...) {
		}
		
		static inline void SetObjectPointer(Object* object, tj::shared::intern::Resource<Object>* rs) {
			object->_resource = rs;
		}

		static std::map< void*, std::wstring> _objects;
};

template<class T> ref<T> GC::Hold(T* x) {
	tj::shared::intern::Resource<T>* rs = new tj::shared::intern::Resource<T>(x);
	SetObjectPointer(x, reinterpret_cast< tj::shared::intern::Resource<Object>* >(rs));
	return rs->Reference();
}

#pragma warning(pop)
#endif