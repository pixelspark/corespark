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
 
 #ifndef _TJSHAREDUI_H
#define _TJSHAREDUI_H

#include <TJShared/include/tjshared.h>
#undef EXPORTED

#ifdef TJSHAREDUI_EXPORTS
	#ifdef TJ_OS_WIN
		#define EXPORTED __declspec(dllexport)
	#else
		#define EXPORTED __attribute__ ((dllexport))
	#endif
#else
	#ifdef TJ_OS_WIN
		#define EXPORTED __declspec(dllimport)
	#else
		#define EXPORTED
	#endif
#endif

#pragma warning(push)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

#include "tjappcore.h"
#include "tjanimation.h"
#include "tjgraphics.h"
#include "tjicon.h"
#include "tjmouse.h"
#include "tjtheme.h"
#include "tjthememgr.h"
#include "tjwnd.h"
#include "tjchildwnd.h"
#include "tjpopupwnd.h"
#include "tjcontextmenu.h"
#include "tjeditwnd.h"
#include "tjpane.h"
#include "tjrootwnd.h"
#include "tjtabwnd.h"
#include "tjbuttonwnd.h"
#include "tjpathwnd.h"
#include "tjgraphwnd.h"
#include "tjloggerwnd.h"
#include "tjprogresswnd.h"
#include "tjtooltipwnd.h"
#include "tjproperty.h"
#include "tjpropertygridwnd.h"
#include "tjsliderwnd.h"
#include "tjsplashwnd.h"
#include "tjsplitterwnd.h"
#include "tjtoolbarwnd.h"
#include "tjlistwnd.h"
#include "tjtreewnd.h"
#include "tjchoicewnd.h"
#include "tjbrowserwnd.h"
#include "tjdialogs.h"
#include "properties/tjproperties.h"

#pragma warning(pop)

#endif