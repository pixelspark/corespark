#ifndef _TJSHARED_H
#define _TJSHARED_H

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED
#endif

// define TJSHARED_MEMORY_TRACE if you want to get a log of allocations/deallocations
#undef TJSHARED_MEMORY_TRACE

#include "tjpch.h"
#pragma warning(push)
#pragma warning(disable: 4251) // about protected members needing dll access

namespace tj {
	namespace shared {
		#include "tjexception.h"
		#include "tjreference.h"
		#include "tjobject.h"
		#include "tjiterator.h"
		#include "tjhash.h"
		#include "tjserializable.h"
		#include "tjrectangle.h"
		#include "tjcolor.h"
		#include "tjutil.h"
		#include "tjfile.h"
		#include "tjflags.h"
		#include "tjrange.h"
		#include "tjthread.h"
		#include "tjcriticalsection.h"
		#include "tjlanguage.h"
		#include "tjlistener.h"
		#include "tjaction.h"
		#include "tjdxutil.h"
		#include "tjarguments.h"
		#include "tjtea.h"
		#include "tjtheme.h"
		#include "tjthememgr.h"
		#include "tjcache.h"
		#include "tjlayout.h"
		#include "tjdialogs.h"
		#include "tjui.h"
		#include "tjchildwnd.h"
		#include "tjcontextmenu.h"
		#include "tjloggerwnd.h"
		#include "tjlog.h"
		#include "tjsplitterwnd.h"
		#include "tjpane.h"
		#include "tjimagebuttonwnd.h"
		#include "tjtabwnd.h"
		#include "tjrootwnd.h"
		#include "tjnotificationwnd.h"
		#include "tjsliderwnd.h"
		#include "tjcore.h"
		#include "tjresourcemgr.h"
		#include "tjtooltip.h" 
		#include "tjedit.h"
		#include "tjgc.h"
		#include "tjproperty.h"
		#include "tjpropertyseparator.h"
		#include "tjreadonlyproperty.h"
		#include "tjvector.h"
		#include "tjcode.h"
		#include "tjcolorproperty.h"
		#include "tjlistproperty.h"
		#include "tjtextproperty.h"
		#include "tjpath.h"
		#include "tjpropertygridwnd.h"
		#include "tjfilewriter.h"
		#include "tjtoolbarwnd.h"
		#include "tjbrowserwnd.h"
		#include "tjprogresswnd.h"
		#include "tjsplash.h"
	}
}

#pragma warning(pop)

#endif