#ifndef _TJLANGUAGE_H
#define _TJLANGUAGE_H

namespace tj {
	namespace shared {
		class EXPORTED Language: public virtual Object {
			public:
				static const wchar_t* Get(std::wstring id);
				static void Load(std::wstring file);
				static void LoadDirectory(std::wstring dir);

				/** Translates menus for a window to the language **/
				static void Translate(HWND wnd);
				static void Translate(HMENU menu);
				virtual ~Language();
				static void Clear();
				Language();

			protected:
				std::map<std::wstring, wchar_t*> _strings;
		};
	}
}

#define TL(id) (tj::shared::Language::Get(std::wstring(L#id)))

#endif