/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJSCRIPT_H
#define _TJSCRIPT_H

#include <TJShared/include/tjshared.h>

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