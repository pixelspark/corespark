#ifndef _TJZIP_H
#define _TJZIP_H

#ifdef TJZIP_EXPORTS
	#ifdef TJ_OS_WIN
		#define TJZIP_EXPORTED __declspec(dllexport)
	#else
		#define TJZIP_EXPORTED
	#endif
#else
	#ifdef TJ_OS_WIN
		#define TJZIP_EXPORTED __declspec(dllimport)
	#else
		#define TJZIP_EXPORTED
	#endif
#endif

#pragma warning(push)
#pragma warning(disable: 4251 4275)

#include <TJShared/include/tjshared.h>

#ifdef TJ_OS_WIN
	#ifndef HZIP_DECLARED
	#define HZIP_DECLARED
	DECLARE_HANDLE(HZIP);
	#endif
#else
	typedef void* HZIP;
	typedef int ZRESULT;
#endif

#include "tjpackage.h"

#pragma warning(pop)

#endif