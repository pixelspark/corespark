#ifndef _TJSHARED_H
#define _TJSHARED_H

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED
#endif

typedef int Time; // in milliseconds
typedef unsigned short Channel;

#include <string>
#include "tjreference.h"
#include "tjexception.h"
#include "tjcode.h"
#include "tjdxutil.h"
#include "tjarguments.h"
#include "tjtea.h"
#include "tjthread.h"
#include "tjtheme.h"
#include "tjthememgr.h"

class EXPORTED GC {
	public:
		static void IncrementLive();
		static void DecrementLive();
		template<typename T> static ref< T > Hold(T* x);
		template<typename T> static ref< T > HoldArray(T* x);
};

template<typename T> ref< T> GC::Hold(T* x) {
	Resource< T,Call<T> >* rs = new Resource<T, Call<T> >(x);
	return rs->Reference();
}

template<typename T> ref< T> GC::HoldArray(T* x) {
	ArrayResource< T,Call<T> >* rs = new ArrayResource<T, Call<T> >(x);
	return rs->Reference();
}

#endif