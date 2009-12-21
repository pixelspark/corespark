#ifndef _TJ_DMX_INTERNAL_H
#define _TJ_DMX_INTERNAL_H

#include <TJShared/include/tjshared.h>

#ifdef TJ_OS_WIN
	#include <TJSharedUI/include/tjsharedui.h>
	#define TJ_DMX_HAS_TJSHAREDUI 1
#endif

#ifdef TJDMXENGINE_EXPORTS
	#ifdef TJ_OS_WIN
		#define DMX_EXPORTED __declspec(dllexport)
	#else
		#define DMX_EXPORTED __attribute__ ((visibility("default")))
	#endif
#else
	#ifdef TJ_OS_WIN
		#define DMX_EXPORTED __declspec(dllimport)
	#else
		#define DMX_EXPORTED
	#endif
#endif

#endif