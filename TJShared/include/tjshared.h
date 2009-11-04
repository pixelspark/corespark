#ifndef _TJSHARED_H
#define _TJSHARED_H

#include "internal/tjpch.h"
#pragma warning(push)
#pragma warning(disable: 4251) // about protected members needing dll access
#pragma warning(disable: 4355) // about 'this' being used in initializers - needed for Animated

#include "tjiterator.h"
#include "tjmixed.h"
#include "tjcrypto.h"
#include "tjlanguage.h"
#include "tjtime.h"
#include "tjutil.h"
#include "tjserializable.h"
#include "tjany.h"
#include "tjrectangle.h"
#include "tjcolor.h"
#include "tjfile.h"
#include "tjthread.h"
#include "tjzone.h"
#include "tjlistener.h"
#include "tjaction.h"
#include "tjlog.h"
#include "tjresourcemgr.h"
#include "tjvector.h"
#include "tjcode.h"
#include "tjsettings.h"
#include "tjendpoint.h"
#include "tjprototype.h"

#pragma warning(pop)
#endif