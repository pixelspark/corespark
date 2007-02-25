#ifndef _TJSHAREDDIALOGS_H
#define _TJSHAREDDIALOGS_H

namespace tj {
	namespace shared {
		class EXPORTED Dialog {
			public:
				static std::wstring AskForSaveFile(HWND owner, std::wstring title, const wchar_t* filter, std::wstring defExt);
				static std::wstring AskForOpenFile(HWND owner, std::wstring title, const wchar_t* filter, std::wstring defExt);
		};
	}
}

#endif