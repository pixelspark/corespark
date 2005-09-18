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
			GC::DecrementLive();
		}

		void AddReference() {
			_rc++;
		}

		void DeleteReference() {
			_rc--;
			if(_rc==0) {
				delete this;
			}
		}

		ref<T,R> Reference() {
			return ref<T,R>(this);
		}

		inline T* GetData() { return _data; }

		T* _data;
	protected:
		Resource(T* x) { 
			_rc = 0;
			_data = x;
			GC::IncrementLive();
		}
  
		void Release() {
			delete _data;
		}

		int _rc;	
		
};


template<class T> class Call {
	friend class ref<T,Call>;

	public:
		Call(Resource<T,Call>* s) {
			if(s==0) throw Exception(L"Tryed to construct a call without subject", ExceptionTypeError);
			_subject = s;
			
			s->AddReference();
		}

		~Call() {
			_subject->DeleteReference();
			_subject = 0;
		}

		inline T* operator->() {
			return _subject->_data;
		}

		inline operator T*() {
			return _subject->_data;
		}

		int operator&() {
			return 0; // no pointers to this class
		}

	protected:
		Resource<T,Call>* _subject;

		Call(const Call<T>& org) {
			_subject = org._subject;
			_subject->AddReference();
		}
};

template<typename T, class R> class ref {
	friend class Resource<T,R>;

	public:
		ref(Resource<T,R>* rx=0) {
			_res = rx;
			if(_res!=0) {
			_res->AddReference();
			}
		}

		ref(const ref<T,R>& org) {
			_res = org._res;
			if(_res!=0) {
				_res->AddReference();
			}
		} 

		template<typename RT, class RR> ref(const ref<RT,RR>& org) {			
			T* rt = dynamic_cast<T*>(org._res->_data);				
			if(rt==0) throw BadCastException();

			_res = reinterpret_cast<Resource<T,R>* >(org._res);
			if(_res!=0) {
				_res->AddReference();
			}
		}

		~ref() {
			if(_res==0) return;
			_res->DeleteReference();
			_res = 0;
		}

		ref<T,R>& operator=(const ref<T,R>& o) {
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

		/*operator T() {
			if(_res==0) {
				throw Exception("Tryed to dereference a null pointer.", ExceptionTypeError);
			}
			return R(_res);
		}*/

		operator ref<const T,R>() {
			return ref<const T,R>((res<const T,R>*)_res);
		}

		ref<const T, R> get_const() {
			return ref<const T,R>((res<const T,R>*)_res);
		}

		operator T&() {
			return *(_res->_data);
		}

		R operator->() {
			return R(_res);
		}

		operator bool() {
			return _res!=0&&_res->_data!=0;
		}

			bool operator==(const ref<T,R>& r) {
		return (r._res==_res);
		}

		ref<T,R> Copy() {
			if(_res==0) throw Exception("Tryed to copy a null reference", ExceptionTypeError);
			return _res->Reference();
		}

		bool IsNull() { return _res==0; }

		template<typename TT, class RR> bool operator==(ref<TT,RR>& r) {
			return (_res==r._res);
		}

		template<typename TT, class RR> bool operator!=(ref<TT,RR>& r) {
			return (_res!=r._res);
		}

		template<class X> bool IsCastableTo() {
			return dynamic_cast<X*>(_res->_data)!=0;	
		}

		Resource<T,R>* _res;
};

#endif
