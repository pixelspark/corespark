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