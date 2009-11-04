#ifndef _TJSHAREDPCH_H
#define _TJSHAREDPCH_H

/** Which platform are we building on? **/
#ifdef __APPLE__
	#ifndef TJ_OS_WIN
		#define TJ_OS_MAC
		#define TJ_OS_POSIX
	#endif
#endif

#ifdef _MSC_VER
	#ifndef TJ_OS_WIN
		#define TJ_OS_WIN
	#endif
#endif

#ifdef TJ_OS_WIN
	// When TJ_OS_WIN is defined, we are building for Windows. This means either Windows XP, Vista or 7
	#define _WIN32_WINNT 0x0700
	#define _WIN32_IE 0x0700
	#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1
	#define _CRT_RAND_S
	#define WIN32_LEAN_AND_MEAN 1

	#include <stdlib.h>
	#include <winsock2.h>
	#include <windows.h>
	#include <ole2.h>
	#include <ws2tcpip.h>
	#include <intrin.h>
#endif

#ifdef TJ_OS_MAC
	// When TJ_OS_MAC is defined, we are building for Mac OS X from version 10.5 onwards. It assumes that
	// pthreads and BSD sockets are present.
	#include <libkern/OSAtomic.h>
	
	#include <CoreFoundation/CFString.h>
#endif

#include <string>
#include <math.h>

#include "../../../Libraries/TinyXML/tinyxml.h"

#ifdef TJSHARED_EXPORTS 
	#ifdef TJ_OS_WIN
		#define EXPORTED __declspec(dllexport)
	#else
		#define EXPORTED __attribute__ ((visibility("default")))
	#endif
#else
	#ifdef TJ_OS_WIN
		#define EXPORTED
	#else
		#define EXPORTED
	#endif
#endif

// define TJSHARED_MEMORY_TRACE here if you want to get a log of allocations/deallocations

namespace tj {
	namespace shared {
		typedef long long int64; // This is equivalent to __int64 on MSVC++
		typedef std::wstring String;
		
		enum ExceptionType {
			ExceptionTypeSevere=0,
			ExceptionTypeError,
			ExceptionTypeWarning,
			ExceptionTypeMessage,
		};
		
		class EXPORTED Exception {
			public:
				Exception(const String& message, ExceptionType type, const char* file="", int line=0);
				virtual ~Exception();
				
				const String& GetMsg() const;
				ExceptionType GetType() const;
				int GetLine() const;
				const char* GetFile() const;
				String ToString() const;
				
			protected:
				String _message;
				ExceptionType _type;
				const char* _file;
				int _line;
		};
		
		#define Throw(msg,t) throw tj::shared::Exception((const wchar_t*)msg,t,(const char*)__FILE__, (int)__LINE__)
	}
}

#include "../tjreference.h"
#include "../tjgc.h"

#endif

