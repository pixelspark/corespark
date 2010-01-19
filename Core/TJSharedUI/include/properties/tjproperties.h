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