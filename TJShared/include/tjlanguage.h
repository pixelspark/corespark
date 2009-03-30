#ifndef _TJLANGUAGE_H
#define _TJLANGUAGE_H

namespace tj {
	namespace shared {
		typedef String LocaleIdentifier;
		class Property;

		class EXPORTED Language: public virtual Object {
			public:
				static const wchar_t* Get(const String& id);
				static void Load(const String& file);
				static void LoadDirectory(const String& dir, const LocaleIdentifier& locale);

				/** Translates menus for a window to the language **/
				#ifdef _WIN32
					static void Translate(HWND wnd);
					static void Translate(HMENU menu);
				#endif

				virtual ~Language();
				static void Clear();
				Language();
				static ref<Property> CreateLanguageProperty(const String& title, LocaleIdentifier* loc);

			protected:
				static void FindLocales(const String& dir);

				static std::vector< LocaleIdentifier > _availableLocales;
				std::map<String, wchar_t*> _strings;
		};
	}
}

#ifdef TJ_OS_WIN
	#define TL(id) (tj::shared::Language::Get(String(L#id)))
#endif

#ifdef TJ_OS_MAC
	#define TL(id) (tj::shared::Wcs(std::string(#id)))
	#warning TL hack on Mac active
#endif

#endif