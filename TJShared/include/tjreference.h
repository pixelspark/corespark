#ifndef _REFERENCE_H
#define _REFERENCE_H

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "../include/tjexception.h"

template<class T> class Call;

class BadCastException: public Exception {
	public:
		BadCastException(): Exception(L"A bad cast was attempted", ExceptionTypeError) {
		}
};

// T must be a pointer, like MyClass*
template< typename T, class R=Call<T> > class ref;

template< typename T, class R=Call<T> > class Resource {
	friend class ref<T,R>;
	friend class Call<T>;
	friend class GC;

	public:
		virtual ~Resource() {
			if(_data==0) {
				throw Exception(L"Resource deleted with null data", ExceptionTypeWarning);
			}
			if(_rc!=0) {
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
			if(_rc==0) {
				delete this;
			}
		}

		CriticalSection& GetLock() {
			return _lock;
		}

		inline ref<T,R> Reference() {
			return ref<T,R>(this);
		}

		inline T* GetData() { return _data; }

		T* _data;
	protected:
		Resource(T* x) { 
			_rc = 0;
			_data = x;
			GC::IncrementLive(sizeof(T));
		}
  
		void Release() {
			delete _data;
		}

		long _rc;	
};

template<class T> class Call {
	friend class ref<T,Call>;

	public:
		inline Call(Resource<T,Call>* s) {
			if(s==0) throw Exception(L"Null pointer exception", ExceptionTypeError);
			_subject = s;
			
			s->AddReference();
		}

		inline ~Call() {
			_subject->DeleteReference();
			_subject = 0;
		}

		inline T* operator->() {
			return dynamic_cast<T*>(_subject->_data);
		}

	protected:
		Resource<T,Call>* _subject;

		inline Call(const Call<T>& org) {
			_subject = org._subject;
			_subject->AddReference();
		}
};

template<typename T, class R> class ref {
	friend class Resource<T,R>;

	public:
		inline ref(Resource<T,R>* rx=0) {
			_res = rx;
			if(_res!=0) {
			_res->AddReference();
			}
		}

		inline ref(const ref<T,R>& org) {
			_res = org._res;
			if(_res!=0) {
				_res->AddReference();
			}
		} 

		template<typename RT, class RR> inline ref(const ref<RT,RR>& org) {	
			if(org._res==0) {
				_res = 0;
			}
			else {
				T* rt = dynamic_cast<T*>(org._res->_data);				
				if(rt==0) throw BadCastException();

				_res = reinterpret_cast<Resource<T,R>* >(org._res);
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

		inline ref<T,R>& operator=(const ref<T,R>& o) {
			Resource<T,R>* old = _res;
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

		inline operator ref<const T,R>() {
			return ref<const T,R>((res<const T,R>*)_res);
		}

		inline ref<const T, R> get_const() {
			return ref<const T,R>((res<const T,R>*)_res);
		}

		inline operator T&() {
			return *(_res->_data);
		}

		inline R operator->() {
			return R(_res);
		}

		inline operator bool() {
			return _res!=0&&_res->_data!=0;
		}

		inline bool operator==(const ref<T,R>& r) {
			return (r._res==_res);
		}

		inline ref<T,R> Copy() {
			if(_res==0) throw Exception("Tried to copy a null reference", ExceptionTypeError);
			return _res->Reference();
		}

		inline bool IsNull() { return _res==0; }

		template<typename TT, class RR> inline bool operator==(ref<TT,RR>& r) {
			return (_res==r._res);
		}

		template<typename TT, class RR> inline bool operator!=(ref<TT,RR>& r) {
			return (_res!=r._res);
		}

		CriticalSection& GetLock() {
			if(_res==0) throw Exception("Tried to lock a null resource", ExceptionTypeError);
			return _res->GetLock();
		}

		template<class X> inline bool IsCastableTo() {
			return dynamic_cast<X*>(_res->_data)!=0;	
		}

		Resource<T,R>* _res;
};

#endif
