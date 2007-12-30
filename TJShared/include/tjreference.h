#ifndef _REFERENCE_H
#define _REFERENCE_H

#include <intrin.h>

namespace tj {
	namespace shared {
		class BadCastException: public Exception {
			public:
				BadCastException(): Exception(L"A bad cast was attempted", ExceptionTypeError) {
				}
		};

		class BadReferenceException: public Exception {
			public:
				BadReferenceException(): Exception(L" A reference error has occurred", ExceptionTypeError) {
				}
		};


		class NullPointerException: public Exception {
			public:
				NullPointerException(): Exception(L"A null pointer was dereferenced", ExceptionTypeError) {
				}
		};

		// T must be a pointer, like MyClass*
		template<typename T> class ref;
		template<typename T> class weak;
		class GC;

		namespace intern {
			class Resource {
				public:
					inline Resource(): _referenceCount(0), _weakReferenceCount(0) {
					}

					~Resource() {
					}

					inline bool AddReference(bool first = false) {
						if(!first && _referenceCount==0) return false;
						InterlockedIncrement(&_referenceCount);
						return true;
					}

					inline long DeleteReference() {
						long old = InterlockedDecrement(&_referenceCount);
						if(old==0 && !IsWeaklyReferenced()) delete this;
						return old;
					}

					inline void AddWeakReference() {
						InterlockedIncrement(&_weakReferenceCount);
					}

					inline void DeleteWeakReference() {
						long old = InterlockedDecrement(&_weakReferenceCount);
						if(old==0 && !IsReferenced()) delete this;						
					}

					inline bool IsReferenced() const {
						return _referenceCount != 0;
					}

					inline bool IsWeaklyReferenced() const {
						return _weakReferenceCount != 0;
					}

				private:
					volatile long _referenceCount;
					volatile long _weakReferenceCount;
			};
		};

		class Object;

		// If _object is null, then resource is considered 0 too and we have a null reference
		template<typename T> class ref {
			friend class intern::Resource;
			friend class GC;
			friend class weak<T>;

			protected:
				/** Called from the GC. object and rx are guaranteed to be non-null **/
				inline ref(T* object, intern::Resource* rx): _resource(rx), _object(object) {
					assert(rx!=0 && object!=0);
					_resource->AddReference(true);
				}

			public:			
				inline ref(): _object(0) {
				}

				inline ref(const weak<T>& wr) {
					if(wr._object==0) {
						_object = 0;
					}
					else {
						if(wr._resource->AddReference()) {
							// object still alive, we are referenced
							_object = wr._object;
							_resource = wr._resource;
						}
						else {
							_object = 0;
						}
					}
				}

				inline ref(Object* object) {
					if(object!=0) {
						/* This exception can be thrown for two reasons:
						* The object wasn't allocated with GC::Hold
						* The objects tries to create a ref<T> from its constructor (in that case, _resource isn't set yet) */
						if(object->_resource==0) {
							throw BadReferenceException();
						}

						_object = dynamic_cast<T*>(object);
						if(_object==0) throw BadCastException();
						_resource = object->_resource;
						_resource->AddReference();
					}
					else {
						_object = 0;
					}
				}

				inline ref(const ref<T>& org): _object(org._object) {
					if(_object!=0) {
						_resource = org._resource;
						_resource->AddReference();
					}
				} 

				template<typename RT> inline ref(const ref<RT>& org) {	
					if(org._object!=0) {
						_object = dynamic_cast<T*>(org._object);
						if(_object==0) throw BadCastException();

						_resource = org._resource;
						_resource->AddReference();
					}
					else {
						_object = 0;
					}
				}

				inline ~ref() {
					Release();
				}

				inline T* GetPointer() {
					return _object;
				}

				inline T* operator->() {
					if(_object==0) throw NullPointerException();
					return _object;
				}

				inline const T* operator->() const {
					if(_object==0) throw NullPointerException();
					return _object;
				}

				inline operator bool() const {
					return _object!=0;
				}

				inline bool operator<(const ref<T>& r) const {
					return r._object < _object;
				}

				inline bool operator>(const ref<T>& r) const {
					return r._object > _object;
				}

				inline ref<T>& operator=(const ref<T>& o) {
					if(o._object==0) {
						Release();
						_object = 0;
					}
					else {
						// Careful: this handles self-assignment
						intern::Resource* res = o._resource;
						res->AddReference();
						Release();
						_object = o._object;
						_resource = res;
					}

					return (*this);
				}

				/***inline ref<T>& operator=(weak<T>& wr) {
					Release();
					_resource = wr._resource;
					
					if(_resource==0 || !_resource->IsReferenced()) {
						_object = 0;
					}
					else {
						_object = wr._object;
						_resource->AddReference();
					}

					return *this;
				}

				inline ref<T>& operator=(const ref<T>& wr) {
					Release();
					_resource = wr._resource;
					_object = wr._object;
					
					if(_resource!=0) {
						_resource->AddReference();
					}

					return *this;
				}

				inline ref<T>& operator=(Object* object) {
					Release();

					if(object!=0) {
						_object = dynamic_cast<T*>(object);
						if(_object!=0) {
							_resource = object->_resource;
							
							if(_resource!=0) {
								_resource->AddReference();
							}
						}
						else {
							throw BadCastException();
						}
					}
					else {
						_object = 0;
						_resource = 0;
					}

					return *this;
				}***/

				// add dynamic casts for pointer comparisons
				template<typename TT> inline bool operator==(const ref<TT>& r) const {
					return _object == r._object;
				}

				template<typename TT> inline bool operator!=(const ref<TT>& r) const {
					return _object != r._object;
				}

				template<class X> inline bool IsCastableTo() const {
					return (_object!=0) && (dynamic_cast<const X*>(_object)!=0);
				}

			private:
				inline void Release() {
					if(_object!=0) {
						if(_resource->DeleteReference()==0) {
							// This was the last reference to the object; release it
							GC::DecrementLive(sizeof(T));
							delete _object;
						}
						_object = 0;
						_resource = 0;
					}
				}

			public:
				T* _object;
				intern::Resource* _resource;
		};

		template<typename T> class weak {
			friend class intern::Resource;
			friend class ref<T>;

			public:
				inline weak(): _object(0) {
				}

				inline weak(ref<T>& org) {
					if(org._object!=0) {
						_object = org._object;
						_resource = org._resource;
						_resource->AddWeakReference();
					}
					else {
						_object = 0;
					}
				} 

				template<typename RT> explicit inline weak(weak<RT>& org) {	
					if(org._object==0) {
						_object = 0;
					}
					else {
						_object = dynamic_cast<T*>(org._object);
						if(_object==0) throw BadCastException();
						
						_resource = org._resource;
						_resource->AddWeakReference();
					}
				}

				inline ~weak() {
					if(_object!=0) {
						_resource->DeleteWeakReference();
					}
				}

				template<typename TT> inline bool operator==(const ref<TT>& r) const {
					return (_object == r._object);
				}

				template<typename TT> inline bool operator!=(const ref<TT>& r) const {
					return (_object != r._object);
				}

				template<typename TT> inline bool operator==(const weak<TT>& r) const {
					return (_object == r._object);
				}

				template<typename TT> inline bool operator!=(const weak<TT>& r) const {
					return (_object != r._object);
				}

				inline bool IsValid() const {
					if(_object==0) {
						return false;
					}
					else {
						return _resource->IsReferenced();	
					}
				}

			private:
				intern::Resource* _resource;
				T* _object;
		};
	}
}

#endif
