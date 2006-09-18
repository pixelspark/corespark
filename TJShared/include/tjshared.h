#ifndef _TJSHARED_H
#define _TJSHARED_H

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED
#endif

#include "tjpch.h"

#pragma warning(push)
#pragma warning(disable: 4251) // about protected members needing dll access

namespace tj {
	namespace shared {
		#include "tjexception.h"
		#include "tjreference.h"
		#include "tjobject.h"
		#include "tjhash.h"
		#include "tjserializable.h"
		#include "tjrectangle.h"
		#include "tjcolor.h"
		#include "tjutil.h"
		#include "tjflags.h"
		#include "tjrange.h"
		#include "tjthread.h"
		#include "tjcriticalsection.h"
		#include "tjlanguage.h"
		#include "tjlistener.h"
		#include "tjcode.h"
		#include "tjdxutil.h"
		#include "tjarguments.h"
		#include "tjtea.h"
		#include "tjtheme.h"
		#include "tjthememgr.h"
		#include "tjcache.h"
		#include "tjlayout.h"
		#include "tjui.h"
		#include "tjchildwnd.h"
		#include "tjcontextmenu.h"
		#include "tjlogger.h"
		#include "tjlog.h"
		#include "tjsplitter.h"
		#include "tjpane.h"
		#include "tjimagebutton.h"
		#include "tjtab.h"
		#include "tjrootwnd.h"
		#include "tjlayeredwnd.h"
		#include "tjnotificationwnd.h"
		#include "tjslider.h"
		#include "tjcore.h"
		#include "tjresourcemgr.h"
		#include "tjtooltip.h" 
		#include "tjedit.h"
		#include "tjgc.h"
		#include "tjproperty.h"
		#include "tjpropertyseparator.h"
		#include "tjreadonlyproperty.h"
		#include "tjcolorproperty.h"
		#include "tjlistproperty.h"
		#include "tjtextproperty.h"
		#include "tjpropertygrid.h"
		#include "tjfilewriter.h"
		#include "tjtoolbarwnd.h"
		#include "tjbrowserwnd.h"
		#include "tjprogresswnd.h"
	}
}

#pragma warning(pop)

#endif