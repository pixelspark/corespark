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
 
 #ifndef _TJPROPERTIES_H
#define _TJPROPERTIES_H

#pragma warning(push)
#pragma warning(disable: 4251) // about protected members needing dll access

#include "tjgenericproperty.h"
#include "tjpropertyseparator.h"
#include "tjtextproperty.h"
#include "tjvectorproperty.h"
#include "tjlistproperty.h"
#include "tjreadonlyproperty.h"
#include "tjcolorproperty.h"
#include "tjfileproperty.h"
#include "tjlinkproperty.h"
#include "tjsettingsmarshal.h"

namespace tj {
	namespace shared {
		class EXPORTED Properties {
			public:
				static ref<Property> CreateLanguageProperty(const String& title, ref<Inspectable> holder, LocaleIdentifier* loc);
				static ref<Property> CreateTypeProperty(const String& name, ref<Inspectable> holder, Any::Type* type);
		};
	}
}

#pragma warning(pop)
#endif