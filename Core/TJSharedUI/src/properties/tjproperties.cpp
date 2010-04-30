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
 
 #include "../../include/tjsharedui.h"
using namespace tj::shared;

ref<Property> Properties::CreateLanguageProperty(const String& title, ref<Inspectable> holder, LocaleIdentifier* lang) {
	assert(lang!=0);
	ref< GenericListProperty<LocaleIdentifier> > pp = GC::Hold(new GenericListProperty<LocaleIdentifier>(title, holder, lang, *lang));

	std::deque<LocaleIdentifier> availableLocales;
	Language::GetAvailableLocales(availableLocales);

	std::deque<LocaleIdentifier>::const_iterator it = availableLocales.begin();
	while(it!=availableLocales.end()) {
		std::wostringstream name;
		name << *it;
		std::string key = "locale_"+Mbs(*it);
		name << L": " << Language::GetLiteral(key);
		pp->AddOption(name.str(), *it);
		++it;
	}

	return pp;
}

ref<Property> Properties::CreateTypeProperty(const String& name, ref<Inspectable> holder, Any::Type* type) {
	assert(type!=0);
	ref< GenericListProperty<Any::Type> > pt = GC::Hold(new GenericListProperty<Any::Type>(name, holder, type, *type));
	pt->AddOption(TL(type_null), Any::TypeNull);
	pt->AddOption(TL(type_integer), Any::TypeInteger);
	pt->AddOption(TL(type_double), Any::TypeDouble);
	pt->AddOption(TL(type_string), Any::TypeString);
	pt->AddOption(TL(type_bool), Any::TypeBool);
	pt->AddOption(TL(type_object), Any::TypeObject);
	pt->AddOption(TL(type_tuple), Any::TypeTuple);
	return pt;
}