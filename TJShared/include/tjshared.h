#ifndef _TJSHARED_H
#define _TJSHARED_H

#ifdef TJSHARED_EXPORTS 
	#define EXPORTED __declspec(dllexport)
#else
	#define EXPORTED
#endif

// define TJSHARED_MEMORY_TRACE if you want to get a log of allocations/deallocations
#undef TJSHARED_MEMORY_TRACE
//#define TJSHARED_MEMORY_TRACE

#include "tjpch.h"
#pragma warning(push)
#pragma warning(disable: 4251) // about protected members needing dll access

#include "tjexception.h"
#include "tjutil.h"
#include "tjreference.h"
#include "tjobject.h"
#include "tjiterator.h"
#include "tjmixed.h"
#include "tjhash.h"
#include "tjnetwork.h"
#include "tjserializable.h"
#include "tjany.h"
#include "tjrectangle.h"
#include "tjcolor.h"
#include "tjutil.h"
#include "tjfile.h"
#include "tjflags.h"
#include "tjrange.h"
#include "tjtimestamp.h"
#include "tjthread.h"
#include "tjlanguage.h"
#include "tjlistener.h"
#include "tjaction.h"
#include "tjdxutil.h"
#include "tjarguments.h"
#include "tjtea.h"
#include "tjcache.h"
#include "tjlayout.h"
#include "tjlog.h"
#include "tjcore.h"
#include "tjresourcemgr.h"
#include "tjgc.h"
#include "tjvector.h"
#include "tjcode.h"
#include "tjfilewriter.h"
#include "tjsettings.h"
#include "tjendpoint.h"
#include "tjtime.h"
#include "ui/tjui.h"
#include "properties/tjproperties.h"

#pragma warning(pop)
#endif