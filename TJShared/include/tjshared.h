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
#include <sstream>
#include <algorithm>
#include <math.h>
#include <vector>

#include "tjutil.h"
#include "tjflags.h"
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
#include "tjcontextmenu.h"
#include "tjlogger.h"
#include "tjlog.h"
#include "tjsplitter.h"
#include "tjpane.h"
#include "tjtab.h"
#include "tjrootwnd.h"
#include "tjslider.h"
#include "tjresourcemgr.h"
#include "tjtooltip.h" 
#include "tjimagebutton.h"
#include "tjedit.h"

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED GC {
	public:
		static void IncrementLive(size_t size=0);
		static void DecrementLive(size_t size=0);
		static long GetLiveCount();
		static long GetSize();
		template<typename T> static ref< T > Hold(T* x);

		static void AddLog(void* id, std::wstring info);
		static void RemoveLog(void* id);

	protected:
		static std::map< void*, std::wstring> _objects;
};

template<typename T> ref<T> GC::Hold(T* x) {
	// get type information
	std::string name = typeid(x).name();

	wchar_t* buf  = new wchar_t[name.length()+2];
	mbstowcs_s(0, buf, name.length()+1, name.c_str(), _TRUNCATE);
	AddLog((void*)x, std::wstring(buf));
	delete[] buf;

	Resource<T>* rs = new Resource<T>(x);
	return rs->Reference();
}

#pragma warning(pop)

#endif