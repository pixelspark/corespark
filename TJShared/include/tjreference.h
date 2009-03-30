#ifndef _REFERENCE_H
#define _REFERENCE_H

namespace tj {
	namespace shared {
		class BadCastException: public Exception {
			public:
				BadCastException(): Exception(L"A bad cast was attempted", ExceptionTypeError) {
				}
		};

		class StrongReferenceException: public Exception {
			public:
				StrongReferenceException(): Exception(L"A null reference tried to become a strong reference", ExceptionTypeError) {
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
		
		namespace intern {
			class Resource;
		}
		
		/**  All classes that want Object functionality should inherit from it virtually:
		 
		 class X: public virtual Object, public OtherClass, public virtual SomeInterface {..};
		 
		 Without the virtual keyword, duplicate base instances of Object could be instantiated, which is not
		 guaranteed to work. Object should be listed first in the list of extended classes.
		 
		 **/
		class EXPORTED Object {
			friend class GC;
			
			public:
				inline Object(): _resource(0) {
				}
				
				struct EXPORTED Notification {}; // useful for empty notifications; then just use Type::Notification
				
				// To make it a polymorphic type
				virtual ~Object() {
				}
				
				virtual void OnCreated();
				
				intern::Resource* _resource;
		};

		// T must be a pointer, like MyClass*
		template<typename T> class ref;
		template<typename T> class weak;
		template<typename T> class strong;
		class GC;
		
		#ifdef TJ_OS_MAC
				typedef int32_t ReferenceCount;
		#endif
				
		#ifdef TJ_OS_WIN
				typedef long ReferenceCount;
		#endif

		namespace intern {			
			class EXPORTED Resource {
				friend class tj::shared::GC;

				public:
					Resource();
					~Resource(); // tjshared.cpp

					inline bool AddReference(bool first = false) {
						if(!first && _referenceCount==0) return false;
						#ifdef TJ_OS_WIN
							_InterlockedIncrement(&_referenceCount);
						#endif
						
						#ifdef TJ_OS_MAC
							OSAtomicAdd32(1, &_referenceCount);
						#endif

						return true;
					}

					inline long DeleteReference() {
						#ifdef TJ_OS_WIN
							ReferenceCount nv = _InterlockedDecrement(&_referenceCount);
						#endif
						
						#ifdef TJ_OS_MAC
							ReferenceCount nv = OSAtomicAdd32(-1, &_referenceCount) ;
						#endif

						if(nv==0 && !IsWeaklyReferenced()) {
							delete this;
						}
						return nv;
					}

					inline void AddWeakReference() {
						#ifdef TJ_OS_WIN
							_InterlockedIncrement(&_weakReferenceCount);
						#endif
												
						#ifdef TJ_OS_MAC
							OSAtomicAdd32(1, &_weakReferenceCount);
						#endif
					}

					inline void DeleteWeakReference() {
						#ifdef TJ_OS_WIN
							ReferenceCount nv = _InterlockedDecrement(&_weakReferenceCount);
						#endif
												
						#ifdef TJ_OS_MAC
							ReferenceCount nv = OSAtomicAdd32(-1, &_weakReferenceCount) ;
						#endif

						if(nv==0 && !IsReferenced()) {
							delete this;	
						}
					}

					inline bool IsReferenced() const {
						return _referenceCount != 0;
					}

					inline bool IsWeaklyReferenced() const {
						return _weakReferenceCount != 0;
					}

					static long GetResourceCount();

				protected:
					volatile ReferenceCount _referenceCount;
					volatile ReferenceCount _weakReferenceCount;
					volatile static ReferenceCount _resourceCount;
			};
		};

		class Object;

		enum Null {
			null = 0,
		};

		// If _object is null, then resource is considered 0 too and we have a null reference
		template<typename T> class ref {
			friend class intern::Resource;
			friend class GC;
			friend class weak<T>;
			friend class strong<T>;

			protected:
				/** Called from the GC. object and rx are guaranteed to be non-null **/
				inline ref(T* object, intern::Resource* rx): _resource(rx), _object(object) {
					assert(rx!=0 && object!=0);
					_resource->AddReference(true);
				}

			public:			
				inline ref(): _object(0) {
				}

				/* This constructor allows assigning and creating references to 'nothing'. Because of the
				ref(Object*) constructor, the =0 syntax will not work. That is why a separate 'Null' type was
				created. If you try to use =0 anyway, the compiler will complain about ambiguity, since it cannot
				choose between converting your '0' to a Null or to a Object*. This is good, since using the ref(Object*)
				constructor with 0 might cause some trouble anyway. Bottom line: always use =null syntax.*/
				inline ref(const Null& n): _object(0) {
				}

				inline ref(const strong<T>& org) {
					if(org._object._object!=0) {
						_object = dynamic_cast<T*>(org._object._object);
						if(_object==0) throw BadCastException();

						_resource = org._object._resource;
						_resource->AddReference();
					}
					else {
						_object = 0;
					}
				} 

				template<typename RT> inline ref(const strong<RT>& org): _object(org._object._object) {
					if(_object!=0) {
						_resource = org._object._resource;
						_resource->AddReference();
					}
				} 

				inline ref(Object* object) {
					if(object!=0) {
						/* This exception can be thrown for two reasons:
						* The object wasn't allocated with GC::Hold
						* The objects tries to create a ref<T> from its constructor (in that case, _resource isn't set yet) */
						_object = dynamic_cast<T*>(object);
						if(_object==0) throw BadCastException();
						_resource = object->_resource;

						if(_resource==0) throw BadReferenceException();
						if(!_resource->IsReferenced()) throw BadReferenceException();

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

				// add dynamic casts for pointer comparisons
				template<typename TT> inline bool operator==(const ref<TT>& r) const {
					return _object == r._object;
				}

				template<typename TT> inline bool operator!=(const ref<TT>& r) const {
					return _object != r._object;
				}

				template<typename TT> inline bool operator==(const strong<TT>& r) const {
					return _object == r._object;
				}

				template<typename TT> inline bool operator!=(const strong<TT>& r) const {
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
							#ifdef TJSHARED_MEMORY_TRACE
							GC::Log(typeid(_object).name(), false);
							#endif
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

		template<typename T> class strong {
			friend class ref<T>;
			friend class weak<T>;

			public:
				inline strong(const ref<T>& t): _object(t) {
					if(!t) {
						throw StrongReferenceException();
					}
				}

				inline strong(const strong<T>& t): _object(t._object) {
					if(!_object) {
						throw StrongReferenceException();
					}
				}

				// This could throw BadCastException
				template<typename TT> inline strong(const strong<TT>& t): _object(t._object) {
				}

				inline ~strong() {
				}

				inline strong<T>& operator=(const strong<T>& o) {
					if(!o._object) {
						throw StrongReferenceException();
					}
					_object = o._object;
					return (*this);
				}

				inline strong<T>& operator=(const ref<T>& o) {
					if(!o) {
						throw StrongReferenceException();
					}
					_object = o;
					return (*this);
				}

				template<typename TT> inline bool operator==(const strong<TT>& r) const {
					return (_object == r._object);
				}

				template<typename TT> inline bool operator!=(const strong<TT>& r) const {
					return (_object != r._object);
				}

				template<typename TT> inline bool operator==(const ref<TT>& r) const {
					return (_object == r._object);
				}

				template<typename TT> inline bool operator!=(const ref<TT>& r) const {
					return (_object != r._object);
				}

				inline bool operator<(const strong<T>& r) const {
					return r._object < _object;
				}

				inline bool operator>(const strong<T>& r) const {
					return r._object > _object;
				}

				inline T* operator->() {
					return _object._object;
				}

				inline const T* operator->() const {
					return _object._object;
				}

				ref<T> _object;
		};

		template<typename T> class weak {
			friend class intern::Resource;
			friend class ref<T>;

			public:
				inline weak(): _object(0) {
				}

				inline weak(const weak<T>& org): _object(org._object) {
					if(_object!=0) {
						_resource = org._resource;
						_resource->AddWeakReference();
					}
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
					Release();
				}

				inline weak<T>& operator=(const weak<T>& o) {
					if(o._object==0) {
						Release();
					}
					else {
						// Careful: this handles self-assignment
						intern::Resource* res = o._resource;
						res->AddWeakReference();
						Release();
						_object = o._object;
						_resource = res;
					}

					return (*this);
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

				template<typename TT> inline bool operator==(const TT* r) const {
					return dynamic_cast<const TT*>(_object) == r;
				}

				inline bool operator<(const weak<T>& r) const {
					return r._object < _object;
				}

				inline bool operator>(const weak<T>& r) const {
					return r._object > _object;
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
				inline void Release() {
					if(_object!=0) {
						_object = 0;
						_resource->DeleteWeakReference();
					}
				}
				intern::Resource* _resource;
				T* _object;
		};
	}
}

#endif
