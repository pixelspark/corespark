#ifndef _TJSCRIPT_H
#define _TJSCRIPT_H

#undef SCRIPT_EXPORTED
#ifdef TJSCRIPT_EXPORTS 
	#define SCRIPT_EXPORTED __declspec(dllexport)
#else
	#define SCRIPT_EXPORTED __declspec(dllimport)
#endif

#pragma warning(push)
#pragma warning(disable: 4251) // protected member needs dll interface
#pragma warning(disable: 4275) // base class needs dll interface

#include <map>
#include <string>
#include <stack>
#include <vector>
#include <deque>

#include "../../TJShared/include/tjshared.h"
#include "tjscriptexception.h"
#include "tjscriptable.h"
#include "tjscriptvalue.h"
#include "tjcompiledscript.h"
#include "tjscripttype.h"
#include "tjscriptcontext.h"
#include "tjscriptthread.h"
#include "tjscriptparameter.h"
#include "tjscriptscope.h"
#include "tjscriptparameterlist.h"
#include "tjscriptobject.h"
#include "types/tjstatictypes.h"
#include "tjscriptiterator.h"

#pragma warning(pop)

#endif