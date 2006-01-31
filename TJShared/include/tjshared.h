#ifndef _TJSHARED_H
#define _TJSHARED_H

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED
#endif

#undef _WIN32_IE
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1

#include <windows.h>
#include <assert.h>
#include <commctrl.h>
#include <map>
#include <string>
#include "tjrange.h"
#include "tjthread.h"
#include "tjcriticalsection.h"
#include "tjreference.h"
#include "tjexception.h"
#include "tjlanguage.h"
#include "tjlistener.h"
#include "tjcode.h"
#include "tjdxutil.h"
#include "tjarguments.h"
#include "tjtea.h"
#include "tjtheme.h"
#include "tjthememgr.h"
#include "tjcache.h"
#include "tjui.h"
#include "tjchildwnd.h"
#include "tjtooltipwnd.h"
#include "tjcontextmenu.h"
#include "tjlogger.h"
#include "tjlog.h"
#include "tjsplitter.h"
#include "tjtab.h"
#include "tjslider.h"
#include "tjresourcemgr.h"

class EXPORTED GC {
	public:
		static void IncrementLive(size_t size=0);
		static void DecrementLive(size_t size=0);
		static long GetLiveCount();
		static long GetSize();
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