#ifndef _TJSHARED_H
#define _TJSHARED_H

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED
#endif

#undef _WIN32_IE
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1

#include <assert.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <list>
#include <iomanip>
#include <fstream>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <atlbase.h>
#include <dshow.h>
#include <Gdiplus.h>

// TODO: move tinyxml to other folder
#include "../../Libraries/tinyxml.h"

namespace tj {
	namespace shared {
		#include "tjserializable.h"
		#include "tjrectangle.h"
		#include "tjcolor.h"
		#include "tjutil.h"
		#include "tjflags.h"
		#include "tjrange.h"
		#include "tjthread.h"
		#include "tjcriticalsection.h"
		#include "tjexception.h"
		#include "tjreference.h"
		#include "tjlanguage.h"
		#include "tjlistener.h"
		#include "tjcode.h"
		#include "tjdxutil.h"
		#include "tjarguments.h"
		#include "tjtea.h"
		#include "tjtheme.h"
		#include "tjthememgr.h"
		#include "tjcache.h"
		#include "tjui.h"
		#include "tjchildwnd.h"
		#include "tjcontextmenu.h"
		#include "tjlogger.h"
		#include "tjlog.h"
		#include "tjsplitter.h"
		#include "tjpane.h"
		#include "tjtab.h"
		#include "tjrootwnd.h"
		#include "tjslider.h"
		#include "tjresourcemgr.h"
		#include "tjtooltip.h" 
		#include "tjimagebutton.h"
		#include "tjedit.h"
		#include "tjgc.h"
	}
}

#endif