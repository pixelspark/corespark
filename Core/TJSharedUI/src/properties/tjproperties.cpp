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