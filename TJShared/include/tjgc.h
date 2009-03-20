#ifndef _TJGC_H
#define _TJGC_H

namespace tj {
	namespace shared {
		class EXPORTED GC {
			public:
				template<typename T> static ref< T > Hold(T* x);
				static void Log(const char* name, bool allocate);

			protected:
				static inline void SetObjectPointer(...) {
				}
				
				static inline void SetObjectPointer(tj::shared::Object* object, tj::shared::intern::Resource* rs) {
					/** There is potential danger here: when SetObjectPointer is called, rs->_referenceCount == 0.
					When a reference is created in OnCreated (which is likely!) and then destroyed, it will see 
					a reference count of 0 and will delete the object. So then, we probably return some bad memory...
					a recipe for trouble. Therefore, this function increments the reference count before doing anything.
					We need to catch errors for OnCreated, because otherwise the reference count is wrong anyway */
					rs->_referenceCount++;
					try {
						object->_resource = rs;
						object->OnCreated();
						rs->_referenceCount--;
					}
					catch(...) {
						rs->_referenceCount--;
						throw;
					}
				}

				static std::map< void*, String> _objects;
		};

		class EXPORTED OutOfMemoryException: public Exception {
			public:
				OutOfMemoryException();
		};

		template<class T> ref<T> GC::Hold(T* x) {
			intern::Resource* rs = new intern::Resource();
			if(rs==0) {
				throw OutOfMemoryException();
			}
			SetObjectPointer(x, rs);
			
			#ifdef TJSHARED_MEMORY_TRACE
				Log(typeid(x).name(),true);
			#endif

			return ref<T>(x, rs);
		}
	}
}

#endif