#ifndef _TJSHARED_H
#define _TJSHARED_H

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED
#endif

#include <string>
#include "tjreference.h"
#include "tjexception.h"
#include "tjcode.h"
#include "tjdxutil.h"

class EXPORTED GC {
	public:
		static void IncrementLive();
		static void DecrementLive();
		template<typename T> static ref< T > Hold(T* x);
};

template<typename T> ref< T> GC::Hold(T* x) {
	Resource< T,Call<T> >* rs = new Resource<T, Call<T> >(x);
	return rs->Reference();
}

#endif