#ifndef _TJSCRIPT_H
#define _TJSCRIPT_H

#include "../../TJShared/include/tjshared.h"

#undef SCRIPT_EXPORTED
#ifdef TJSCRIPT_EXPORTS 
	#ifdef TJ_OS_WIN
		#define SCRIPT_EXPORTED __declspec(dllexport)
	#else
		#define SCRIPT_EXPORTED __attribute__((visibility("default")))
	#endif
#else
	#ifdef TJ_OS_WIN
		#define SCRIPT_EXPORTED __declspec(dllimport)
	#else
		#define SCRIPT_EXPORTED
	#endif
#endif

#pragma warning(push)
#pragma warning(disable: 4251) // protected member needs dll interface
#pragma warning(disable: 4275) // base class needs dll interface

#include <map>
#include <string>
#include <stack>
#include <vector>
#include <deque>

#include "tjscriptexception.h"
#include "tjscriptable.h"
#include "tjscriptvalue.h"
#include "tjcompiledscript.h"
#include "tjscripttype.h"
#include "tjscriptcontext.h"
#include "tjscriptthread.h"
#include "tjscriptscope.h"
#include "tjscriptparameter.h"
#include "tjscriptparameterlist.h"
#include "tjscriptobject.h"
#include "tjscriptdelegate.h"
#include "tjscriptiterator.h"

#pragma warning(pop)

#endif