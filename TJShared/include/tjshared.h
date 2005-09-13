#ifndef _TJSHARED_H
#define _TJSHARED_H

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED __declspec(dllimport)
#endif

#include <string>
#include "tjreference.h"
#include "tjexception.h"

class EXPORTED  GC {
	public:
		GC(); // do not instantiate!
		~GC();
		template<typename T> static ref< T > Hold(T* x);
		static unsigned int _live;
};

template<typename T> ref< T> GC::Hold(T* x) {
	Resource< T,Call<T> >* rs = new Resource<T, Call<T> >(x);
	return rs->Reference();
}

#endif