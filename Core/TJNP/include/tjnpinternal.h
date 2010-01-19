#ifndef _TJ_NP_INTERNAL_H
#define _TJ_NP_INTERNAL_H

#include <TJShared/include/tjshared.h>

#ifdef TJNP_EXPORTS
	#ifdef TJ_OS_WIN
		#define NP_EXPORTED __declspec(dllexport)
	#else
		#define NP_EXPORTED __attribute__ ((visibility("default")))
	#endif
#else
	#ifdef TJ_OS_WIN
		#define NP_EXPORTED __declspec(dllimport)
	#else
		#define NP_EXPORTED
	#endif
#endif

#endif