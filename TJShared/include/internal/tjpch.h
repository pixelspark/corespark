#ifndef _TJSHAREDPCH_H
#define _TJSHAREDPCH_H

/** Which platform are we building on? **/
#ifdef __APPLE__
	#define TJ_OS_MAC
#endif

#ifdef WIN32
	#define TJ_OS_WIN
#endif

#ifdef TJ_OS_WIN
	#define _WIN32_WINNT 0x0700
	#define _WIN32_IE 0x0700
	#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1
	#define _CRT_RAND_S 1
	#define WIN32_LEAN_AND_MEAN 1

	#include <winsock2.h>
	#include <windows.h>
	#include <ole2.h>
	#include <ws2tcpip.h>
	#include <intrin.h>
#endif

#ifdef TJ_OS_MAC
	#include <libkern/OSAtomic.h>
	#include <stdlib.h>
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

#include "../../../Libraries/TinyXML/tinyxml.h"

#ifdef TJSHARED_EXPORTS 
	#ifdef TJ_OS_WIN
		#define EXPORTED __declspec(dllexport)
	#else
		#define EXPORTED
	#endif
#else
	#define EXPORTED
#endif

// define TJSHARED_MEMORY_TRACE if you want to get a log of allocations/deallocations
#undef TJSHARED_MEMORY_TRACE
//#define TJSHARED_MEMORY_TRACE

namespace tj {
	namespace shared {
		typedef std::wstring String;
		
		#ifdef TJ_OS_MAC
			template<typename T> inline T& min(T& a, T& b) {
				return (a<b) ? a : b;
			}
		
			template<typename T> inline T& max(T& a, T& b) {
				return (a>b) ? a : b;
			}
		
			template<typename T> inline const T& min(const T& a, const T& b) {
				return (a<b) ? a : b;
			}
			
			template<typename T> inline const T& max(const T& a, const T& b) {
				return (a>b) ? a : b;
			}
		#endif
	}
}
#endif

