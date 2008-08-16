#ifndef _TJSHAREDPCH_H
#define _TJSHAREDPCH_H

#define _WIN32_WINNT 0x0700
#define _WIN32_IE 0x0700
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1
#define _CRT_RAND_S 1
#define WIN32_LEAN_AND_MEAN 1

#include <winsock2.h>
#include <windows.h>
#include <winspool.h>
#include <ole2.h>
#include <ws2tcpip.h>
#include <Gdiplus.h>

namespace tj {
	namespace shared {
		namespace graphics = ::Gdiplus;
	}
}

#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <set>
#include <list>
#include <iomanip>
#include <fstream>
#include <deque>
#include <math.h>

// TODO: move tinyxml to other folder
#include "../../Libraries/tinyxml.h"

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED
#endif

// define TJSHARED_MEMORY_TRACE if you want to get a log of allocations/deallocations
#undef TJSHARED_MEMORY_TRACE
//#define TJSHARED_MEMORY_TRACE

#endif