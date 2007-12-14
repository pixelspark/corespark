#ifndef _REFERENCE_H
#define _REFERENCE_H

namespace tj {
	namespace shared {
		class BadCastException: public Exception {
			public:
				BadCastException(): Exception(L"A bad cast was attempted", ExceptionTypeError) {
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
			template< typename T > class Resource {
				friend class tj::shared::ref<T>;
				friend class tj::shared::weak<T>;
				friend class tj::shared::GC;

				public:
					virtual ~Resource() {
						#ifdef _DEBUG
						if(_rc!=0 || _weakrc!=0) {
								throw Exception(L"Resource deleted while still referenced!",ExceptionTypeWarning);
						}
						#endif

						this->Release();
						GC::DecrementLive(sizeof(T));
					}

					inline void AddReference() {
						InterlockedIncrement(&_rc);
					}

					inline void DeleteReference() {
						InterlockedDecrement(&_rc);
						
						if(_rc==0) {
							// If there are no outstanding weak references, we can delete ourselves
							if(_weakrc==0) {
								#ifdef TJSHARED_MEMORY_TRACE
								GC::Log(typeid(this).name(), false);
								#endif

								delete this;
							}
							else {
								// There are outstanding weak references, but at least we can delete the object
								Release();
							}
						}
					}

					inline void AddWeakReference() {
						InterlockedIncrement(&_weakrc);
					}

					inline void DeleteWeakReference() {
						InterlockedDecrement(&_weakrc);

						if(_rc==0 && _weakrc==0) {
							delete this;
						}
					}

					inline ref<T> Reference() {
						return ref<T>(this);
					}

					inline weak<T> WeakReference() {
						return weak<T>(this);
					}

					inline T* GetData() { return _data; }

					T* _data;
				protected:
					Resource(T* x) { 
						_rc = 0;
						_weakrc = 0;
						_data = x;
						GC::IncrementLive(sizeof(T));
					}
			  
					void Release() {
						if(_data!=0) {
							T* temp = _data;
							_data = 0;
							delete temp;
						}
					}

					long _rc;
					long _weakrc;
			};
		}

		template<typename T> class ref {
			friend class intern::Resource<T>;

			public:
				inline ref(intern::Resource<T>* rx=0) {
					_res = rx;
					if(_res!=0) {
						_res->AddReference();
					}
				}

				inline ref(const ref<T>& org) {
					_res = org._res;
					if(_res!=0) {
						_res->AddReference();
					}
				} 

				template<typename RT> inline ref(const ref<RT>& org) {	
					if(org._res==0) {
						_res = 0;
					}
					else {
						T* rt = dynamic_cast<T*>(org._res->_data);				
						if(rt==0) throw BadCastException();

						_res = reinterpret_cast<tj::shared::intern::Resource<T>* >(org._res);
						if(_res!=0) {
							_res->AddReference();
						}
					}
				}

				inline ~ref() {
					if(_res==0) return;
					_res->DeleteReference();
					///_res = 0;
				}

				inline ref<T>& operator=(const ref<T>& o) {
					tj::shared::intern::Resource<T>* old = _res;
					_res = o._res;

					if(_res!=0) {
						_res->AddReference();
					}
					if(old!=0) {
						old->DeleteReference();
					}

					return (*this);
				}

				inline T* GetPointer() {
					if(_res->_data==0) return 0;
					return dynamic_cast<T*>(_res->_data);
				}

				inline operator ref<const T>() {
					return ref<const T>((res<const T>*)_res);
				}

				inline ref<const T> get_const() {
					return ref<const T>((res<const T>*)_res);
				}

				inline operator T&() {
					if(_res->_data==0) throw NullPointerException();
					return *(_res->_data);
				}

				inline T* operator->() {
					if(_res->_data==0) throw NullPointerException();
					///return dynamic_cast<T*>(_res->_data);
					return _res->_data;
				}

				inline const T* operator->() const {
					if(_res->_data==0) throw NullPointerException();
					///return dynamic_cast<const T*>(_res->_data);
					return _res->_data;
				}

				inline operator bool() const {
					return _res!=0&&_res->_data!=0;
				}

				inline bool operator<(const ref<T>& r) const {
					return (r._res<_res);
				}

				inline bool operator>(const ref<T>& r) const {
					return (r._res>_res);
				}

				// add dynamic casts for pointer comparisons
				template<typename TT> inline bool operator==(const ref<TT>& r) const {
					return (_res==r._res);
				}

				template<typename TT> inline bool operator!=(const ref<TT>& r) const {
					return (_res!=r._res);
				}

				template<class X> inline bool IsCastableTo() const {
					return dynamic_cast<const X*>(_res->_data)!=0;	
				}

				intern::Resource<T>* _res;
		};

		template<typename T> class weak {
			friend class tj::shared::intern::Resource<T>;

			public:
				inline weak(tj::shared::intern::Resource<T>* rx=0) {
					_res = rx;
					if(_res!=0) {
					_res->AddWeakReference();
					}
				}

				inline weak(const ref<T>& org) {
					_res = org._res;
					if(_res!=0) {
						_res->AddWeakReference();
					}
				} 

				template<typename RT> inline weak(const weak<RT>& org) {	
					if(org._res==0) {
						_res = 0;
					}
					else {
						T* rt = dynamic_cast<T*>(org._res->_data);				
						if(rt==0) throw BadCastException();

						_res = reinterpret_cast<tj::shared::intern::Resource<T>* >(org._res);
						if(_res!=0) {
							_res->AddWeakReference();
						}
					}
				}

				inline ~weak() {
					if(_res==0) return;
					_res->DeleteWeakReference();
					_res = 0;
				}

				inline weak<T>& operator=(const ref<T>& o) {
					tj::shared::intern::Resource<T>* old = _res;
					_res = o._res;

					if(_res!=0) {
						_res->AddWeakReference();
					}
					if(old!=0) {
						old->DeleteWeakReference();
					}

					return (*this);
				}

				inline operator weak<const T>() {
					return weak<const T>((res<const T>*)_res);
				}

				inline weak<const T> get_const() {
					return weak<const T>((res<const T>*)_res);
				}

				inline operator ref<T>() {
					return Reference();
				}

				inline bool operator==(const ref<T>& r) {
					return (r._res==_res);
				}

				template<typename TT> inline bool operator==(ref<TT>& r) {
					return (_res==r._res);
				}

				template<typename TT> inline bool operator!=(ref<TT>& r) {
					return (_res!=r._res);
				}

				template<typename TT> inline bool operator==(const weak<TT>& r) {
					return (_res==r._res);
				}

				template<typename TT> inline bool operator!=(const weak<TT>& r) {
					return (_res!=r._res);
				}

				template<class X> inline bool IsCastableTo() {
					if(_res==0 || _res->_data == 0) return false;
					return dynamic_cast<X*>(_res->_data)!=0;	
				}

				inline ref<T> Reference() {
					if(_res!=0 && _res->_rc>0) {
						return _res->Reference();
					}
					return ref<T>(0);
				}

				inline bool IsValid() const {
					return _res!=0 && _res->_rc>0;
				}

				tj::shared::intern::Resource<T>* _res;
		};
	}
}

#endif
