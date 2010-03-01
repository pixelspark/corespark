#ifndef _REFERENCE_H
#define _REFERENCE_H

#include "tjsharedinternal.h"
#include <deque>

#ifdef TJSHARED_MEMORY_TRACE
	#include <map>
#endif

namespace tj {
	namespace shared {
		template<typename T> class ref;
		template<typename T> class weak;
		template<typename T> class strong;
		class GC;
		class CriticalSection;
		class Recycleable;
		class RecycleBin;
		
		namespace intern {
			class Resource;
		}
		
		#ifdef TJ_OS_POSIX
			typedef int32_t ReferenceCount;
		#endif
				
		#ifdef TJ_OS_WIN
			typedef long ReferenceCount;
		#endif
		
		/** A BadCastException is thrown whenever an attempt is made to cast a reference 
		to another type, to which it cannot be casted. If you are unsure whether you can
		cast a particular object to another type (casting to a known superclass is safe,
		but casting to a subclass, for instance, is not), you should check using the
		IsCastable<Q> method on ref<Q>, which will return true if the object can be safely
		casted to the designated type Q. **/
		class BadCastException: public Exception {
			public:
				BadCastException();
				virtual ~BadCastException();
		};
		
		/** A StrongReferenceException is thrown whenever an attempt is made to assign
		a strong reference to null; strong references can never be null. **/
		class StrongReferenceException: public Exception {
			public:
				StrongReferenceException();
				virtual ~StrongReferenceException();
		};
		
		/** The BadReferenceException is thrown whenever an attempt is made to convert
		a regular pointer to an object into a reference and fails, because the object
		is not a subclass of Object (which is required for this operation to work, because
		it contains a pointer to the Resource object associated with an object) or
		when the object has not been fully created yet. Also, this exception is thrown 
		whenever an attempt is made to convert a pointer to an object that is not managed
		by the memory management system to a reference.
		 
		This exception will, for instance, be thrown if you try to use the 'this' pointer
		of an Object-derived class within its constructor. When still in the constructor,
		the pointer to the Resource object has not yet been set in the Object class, so
		conversion from the this pointer to a reference is not possible. To overcome this
		problem, the memory manager calls the OnCreated method on your Object, in which
		you can safely convert the this pointer to a reference. **/
		class BadReferenceException: public Exception {
			public:
				BadReferenceException();
				virtual ~BadReferenceException();
		};
		
		/** The NullPointerException is thrown whenever a method is called on a reference
		that is null. Strong references never throw this exception when calling a method,
		but only check for nullity once (on construction). If a strong reference is 
		initialized with null, it throws a StrongReferenceException. **/
		class NullPointerException: public Exception {
			public:
				NullPointerException();
				virtual ~NullPointerException();
		};
		
		class EXPORTED OutOfMemoryException: public Exception {
			public:
				OutOfMemoryException();
				virtual ~OutOfMemoryException();
		};
		
		template<class T> class Sortable {
			public:
				virtual ~Sortable() {
				}
				
				virtual bool SortsAfter(const T& other) const = 0;
		};
		
		/**  All classes that want Object functionality should inherit from it virtually:
		 
		 class X: public virtual Object, public OtherClass, public virtual SomeInterface {..};
		 
		 Without the virtual keyword, duplicate base instances of Object could be instantiated, which is not
		 guaranteed to work. Object should be listed first in the list of extended classes.
		 **/
		class EXPORTED Object {
			friend class GC;
			
			public:
				Object();
				virtual ~Object();
				virtual void OnCreated();
				
				struct EXPORTED Notification {}; // useful for empty notifications; then just use Type::Notification
				intern::Resource* _resource;
		};

		namespace intern {			
			class EXPORTED Resource {
				friend class tj::shared::GC;

				public:
					Resource();
					virtual ~Resource(); // tjcore.cpp

					inline bool AddReference(bool first = false) {
						if(!first && _referenceCount==0) return false;
						#ifdef TJ_OS_WIN
							_InterlockedIncrement(&_referenceCount);
						#endif

						#ifdef TJ_OS_MAC
							OSAtomicAdd32(1, &_referenceCount);
						#endif

						#ifdef TJ_OS_LINUX
							//__sync_add_and_fetch(&_referenceCount, 1);
							_referenceCount += 1;
						#endif
						
						return true;
					}
					
					/** Decrements the reference count and returns true if this was 
					the last reference (note that there could still be weak references!) **/
					inline bool DeleteReference() {
						#ifdef TJ_OS_WIN
							ReferenceCount nv = _InterlockedDecrement(&_referenceCount);
						#endif

						#ifdef TJ_OS_MAC
							ReferenceCount nv = OSAtomicAdd32(-1, &_referenceCount) ;
						#endif

						#ifdef TJ_OS_LINUX
							ReferenceCount nv = _referenceCount-1; 
							_referenceCount -= 1;
						#endif
						
						return nv==0;
					}
					
					inline void AddWeakReference() {
						#ifdef TJ_OS_WIN
							_InterlockedIncrement(&_weakReferenceCount);
						#endif
						
						#ifdef TJ_OS_MAC
							OSAtomicAdd32(1, &_weakReferenceCount);
						#endif

						#ifdef TJ_OS_LINUX
							//__sync_add_and_fetch(&_weakReferenceCount,1);
							_weakReferenceCount += 1;
						#endif
					}

					/** Decrements the weak reference count and returns true if this was the last weak
					reference (and there were no other 'real' references) **/
					inline bool DeleteWeakReference() {
						#ifdef TJ_OS_WIN
							ReferenceCount nv = _InterlockedDecrement(&_weakReferenceCount);
						#endif

						#ifdef TJ_OS_MAC
							ReferenceCount nv = OSAtomicAdd32(-1, &_weakReferenceCount) ;
						#endif

						#ifdef TJ_OS_LINUX
							//ReferenceCount nv = __sync_sub_and_fetch(&_weakReferenceCount, 1);
							ReferenceCount nv = _weakReferenceCount-1;
							_weakReferenceCount -= 1;
						#endif
						
						return (nv==0 && !IsReferenced());
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
			
			class EXPORTED RecycleableResource: public Resource {
				public:
					RecycleableResource(RecycleBin& rb);
					virtual ~RecycleableResource();
				
					RecycleBin& _bin;
			};
		}
		
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
			
			template<class Q> inline bool operator<(const ref<Q>& r) const {
				if(r.IsCastableTo< Sortable<T> >()) {
					return ref< Sortable<T> >(r)->SortsAfter(*_object);
				}
				else {
					Sortable<Q>* s = dynamic_cast< Sortable<Q>* >(_object);
					if(s!=0) {
						return !s->SortsAfter(*(r._object));
					}
					else {
						return _object < r._object;
					}
				}
			}
			
			template<class Q> inline bool operator>(const ref<Q>& r) const {
				return this->operator< <Q>(r);
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
			inline void Release();
			
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
				
				template<class X> inline bool IsCastableTo() const {
					return (_object._object!=0) && (dynamic_cast<const X*>(_object._object)!=0);
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
					if(_resource->DeleteWeakReference()) {
						delete _resource;
						_resource = 0;
					}
				}
			}
			
			intern::Resource* _resource;
			T* _object;
		};
		
		/** The Recycler mechanism allows small objects that can be used multiple times to be 'recycled';
		 when the memory management system detects that an object that is Recycleable can be deleted,
		 it will not delete the object but instead add it to a list of objects to be re-used (if the 
		 'recycle bin' is not yet filled with enough objects). **/
		class EXPORTED Recycleable: public virtual Object {
			public:
				virtual ~Recycleable();
				virtual void OnRecycle();
				virtual void OnReuse();
		};
		
		class EXPORTED RecycleBin {
			public:
				RecycleBin();
				virtual ~RecycleBin();
				virtual void Reuse(Recycleable* rc);
				virtual Recycleable* Get();
				static unsigned int GetTrashObjectCount();
			
			private:
				bool WantsToRecycle() const;
				unsigned int _limit;
				CriticalSection* _lock;
				std::deque<Recycleable*> _bin;
				static unsigned int _totalObjectCount;
		};
		
		class EXPORTED GC {
			public:
				template<typename T> static ref<T> Hold(T* x);
				template<typename T> static ref<T> Hold(T* x, RecycleBin& rb);
				template<typename T> static ref<T> HoldRecycled(T* x);
			
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
				
			#ifdef TJSHARED_MEMORY_TRACE
				static std::map< void*, String> _objects;
			#endif
		};
		
		template<class T> class Recycler {
			public:
				static inline strong<T> Create() {
					Recycleable* rc = _bin.Get();
					if(rc!=0) {
						T* object = dynamic_cast<T*>(rc);
						if(object==0) {
							// Something went wrong; for some reason, a wrong object was added to the list.
							delete rc->_resource;
							delete rc;
							return GC::Hold(new T(), _bin);
						}
						
						return GC::HoldRecycled(object);
					}
					return GC::Hold(new T(), _bin);
				}
			
			protected:
				static RecycleBin _bin;
		};
		
		/** Implementations of templated methods **/
		template<class T> RecycleBin Recycler<T>::_bin;
		
		template<class T> inline ref<T> GC::HoldRecycled(T* x) {
			Recycleable* rc = dynamic_cast<Recycleable*>(x);
			if(rc==0) {
				throw BadReferenceException();
			}
			
			if(rc->_resource==0) {
				throw BadReferenceException();
			}
			
			if(rc->_resource->_referenceCount!=0 || rc->_resource->_weakReferenceCount!=0) {
				throw BadReferenceException();
			}
			
			rc->_resource->_referenceCount = 1;
			try {
				rc->OnReuse();
				rc->_resource->_referenceCount = 0;
			}
			catch(...) {
				rc->_resource->_referenceCount = 0;
				throw;
			}
			
			#ifdef TJSHARED_MEMORY_TRACE
				Log(typeid(x).name(),true);
			#endif
			
			return ref<T>(x, rc->_resource);
		}
		
		template<class T> inline ref<T> GC::Hold(T* x) {
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
		
		template<class T> inline ref<T> GC::Hold(T* x, RecycleBin& rb) {
			/* assert(dynamic_cast<Recycleable*>(x)!=0); */
			intern::RecycleableResource* rs = new intern::RecycleableResource(rb);
			if(rs==0) {
				throw OutOfMemoryException();
			}
			
			SetObjectPointer(x, rs);
			
			#ifdef TJSHARED_MEMORY_TRACE
				Log(typeid(x).name(),true);
			#endif
			
			return ref<T>(x, dynamic_cast<intern::Resource*>(rs));
		}
								
		template<class T> inline void ref<T>::Release() {
			if(_object!=0) {
				if(_resource->DeleteReference()) {
					// This was the last reference to the object; release it
					#ifdef TJSHARED_MEMORY_TRACE
						GC::Log(typeid(_object).name(), false);
					#endif
					
					/** Check if there are still weak references; if there are, just
					 delete the object. If there are none, try to recycle the object **/
					if(_resource->IsWeaklyReferenced()) {
						delete _object;
					}
					else {
						intern::RecycleableResource* rrc = dynamic_cast<intern::RecycleableResource*>(_resource);
						if(rrc!=0) {
							rrc->_bin.Reuse(dynamic_cast<Recycleable*>(_object));
						}
						else {
							delete _object;
							delete _resource;
						}
					}
					
				}
				_object = 0;
				_resource = 0;
			}
		}
	}
}

#endif
