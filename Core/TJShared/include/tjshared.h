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
 
 #ifndef _TJSHARED_H
#define _TJSHARED_H

#include "tjsharedinternal.h"
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