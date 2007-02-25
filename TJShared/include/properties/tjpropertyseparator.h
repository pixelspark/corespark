#ifndef _TJPROPERTYSEPARATOR_H
#define _TJPROPERTYSEPARATOR_H

namespace tj {
	namespace shared {
		class EXPORTED PropertySeparator: public Property {
			public:
				PropertySeparator(std::wstring group);
				virtual ~PropertySeparator();
				virtual std::wstring GetValue();
				virtual HWND GetWindow();
				virtual HWND Create(HWND parent);
				virtual void Changed();
				virtual void Update();
				virtual int GetHeight();
		};
	}
}

#endif