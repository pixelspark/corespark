#ifndef _TJ_DB_INTERNAL_H
#define _TJ_DB_INTERNAL_H

#include <TJShared/include/tjshared.h>
using tj::shared::ref;

#ifdef TJDB_EXPORTS
	#ifdef TJ_OS_WIN
		#define DB_EXPORTED __declspec(dllexport)
	#else
		#define DB_EXPORTED
	#endif
#else
	#ifdef TJ_OS_WIN
		#define DB_EXPORTED __declspec(dllimport)
	#else
		#define DB_EXPORTED
	#endif
#endif

#endif