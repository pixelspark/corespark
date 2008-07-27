#ifndef _TJPCH_H
#define _TJPCH_H

#ifdef _WIN32
	#undef _WIN32_IE
	#undef _WIN32_WINNT
	#define _WIN32_WINNT 0x0600
	#define _WIN32_IE 0x0600
	#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1
	#define _CRT_RAND_S 1

	#include <winsock2.h>
	#include <windows.h>
	#include <Gdiplus.h>

	namespace tj {
		namespace shared {
			namespace graphics = ::Gdiplus;
		}
	}
#endif

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