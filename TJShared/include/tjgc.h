#ifndef _TJGC_H
#define _TJGC_H

namespace tj {
	namespace shared {
		class EXPORTED GC {
			public:
				static void IncrementLive(size_t size=0);
				static void DecrementLive(size_t size=0);
				static long GetLiveCount();
				static long GetSize();
				template<typename T> static ref< T > Hold(T* x);
				static void Log(const char* name, bool allocate);

			protected:
				static inline void SetObjectPointer(...) {
				}
				
				static inline void SetObjectPointer(Object* object, intern::Resource* rs) {
					object->_resource = rs;
					object->OnCreated();
				}

				static std::map< void*, std::wstring> _objects;
		};

		template<class T> ref<T> GC::Hold(T* x) {
			intern::Resource* rs = new intern::Resource();
			SetObjectPointer(x, rs);
			
			#ifdef TJSHARED_MEMORY_TRACE
				Log(typeid(x).name(),true);
			#endif

			GC::IncrementLive(sizeof(T));
			return ref<T>(x, rs);
		}
	}
}

#endif