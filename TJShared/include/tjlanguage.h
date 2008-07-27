#ifndef _TJLANGUAGE_H
#define _TJLANGUAGE_H

namespace tj {
	namespace shared {
		typedef std::wstring LocaleIdentifier;
		class Property;

		class EXPORTED Language: public virtual Object {
			public:
				static const wchar_t* Get(const std::wstring& id);
				static void Load(const std::wstring& file);
				static void LoadDirectory(const std::wstring& dir, const LocaleIdentifier& locale);

				/** Translates menus for a window to the language **/
				#ifdef _WIN32
					static void Translate(HWND wnd);
					static void Translate(HMENU menu);
				#endif

				virtual ~Language();
				static void Clear();
				Language();
				static ref<Property> CreateLanguageProperty(const std::wstring& title, LocaleIdentifier* loc);

			protected:
				static void FindLocales(const std::wstring& dir);

				static std::vector< LocaleIdentifier > _availableLocales;
				std::map<std::wstring, wchar_t*> _strings;
		};
	}
}

#define TL(id) (tj::shared::Language::Get(std::wstring(L#id)))

#endif