#ifndef _TJ_SCOUT_INTERNAL_H
#define _TJ_SCOUT_INTERNAL_H

#ifdef TJSCOUT_EXPORTS
	#define SCOUT_EXPORTED __declspec(dllexport)
#else
	#define SCOUT_EXPORTED
#endif

#include <TJShared/include/tjshared.h>

#endif