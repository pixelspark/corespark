#ifndef _REFERENCE_H
#define _REFERENCE_H

template<class T> class Call;

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

template< typename T > class Resource {
	friend class ref<T>;
	friend class weak<T>;
	friend class GC;

	public:
		virtual ~Resource() {
			if(_data==0) {
				throw Exception(L"Resource deleted with null data", ExceptionTypeWarning);
			}
			if((_rc+_weakrc)!=0) {
					throw Exception(L"Resource deleted while still referenced!",ExceptionTypeWarning);
			}

			this->Release();
			GC::DecrementLive(sizeof(T));
			GC::RemoveLog((void*)_data);
		}

		inline void AddReference() {
			InterlockedIncrement(&_rc);
		}

		inline void DeleteReference() {
			InterlockedDecrement(&_rc);
			if(_rc+_weakrc==0) {
				delete this;
			}
			else if(_rc==0) {
				Release();
			}
		}

		inline void AddWeakReference() {
			InterlockedIncrement(&_weakrc);
		}

		inline void DeleteWeakReference() {
			InterlockedDecrement(&_weakrc);
			if(_rc+_weakrc==0) {
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
			delete _data;
			_data = 0;
		}

		long _rc;
		long _weakrc;
};

template<typename T> class ref {
	friend class Resource<T>;

	public:
		inline ref(Resource<T>* rx=0) {
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

				_res = reinterpret_cast<Resource<T>* >(org._res);
				if(_res!=0) {
					_res->AddReference();
				}
			}
		}

		inline ~ref() {
			if(_res==0) return;
			_res->DeleteReference();
			_res = 0;
		}

		inline ref<T>& operator=(const ref<T>& o) {
			Resource<T>* old = _res;
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
			return _res->_data;
		}

		inline operator bool() {
			return _res!=0&&_res->_data!=0;
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

		template<class X> inline bool IsCastableTo() {
			return dynamic_cast<X*>(_res->_data)!=0;	
		}

		Resource<T>* _res;
};

template<typename T> class weak {
	friend class Resource<T>;

	public:
		inline weak(Resource<T>* rx=0) {
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

				_res = reinterpret_cast<Resource<T>* >(org._res);
				if(_res!=0) {
					_res->AddReference();
				}
			}
		}

		inline ~weak() {
			if(_res==0) return;
			_res->DeleteWeakReference();
			_res = 0;
		}

		inline ref<T>& operator=(const ref<T>& o) {
			Resource<T>* old = _res;
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

		inline operator bool() {
			return _res!=0&&_res->_rc>0;
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

		template<class X> inline bool IsCastableTo() {
			return dynamic_cast<X*>(_res->_data)!=0;	
		}

		inline ref<T> Reference() {
			if(_res && _res->_rc>0) {
				return _res->Reference();
			}
			return ref<T>(0);
		}

		Resource<T>* _res;
};

#endif
