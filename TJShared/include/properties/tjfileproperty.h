#ifndef _TJFILEPROPERTY_H
#define _TJFILEPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED FilePropertyWnd: public ChildWnd {
			public:
				FilePropertyWnd(std::wstring name, std::wstring* path, const wchar_t* filter);
				virtual ~FilePropertyWnd();
				virtual void Paint(Gdiplus::Graphics& g);

			protected:
				virtual void OnMouse(MouseEvent me, Pixels x, Pixels y);
				std::wstring* _path;
				std::wstring _name;
				Icon _linkIcon;
				const wchar_t* _filter;
		};

		class EXPORTED FileProperty: public Property {
			public:
				FileProperty(std::wstring name, std::wstring* path, const wchar_t* filter=L"*.*\0\0\0");
				virtual ~FileProperty();
				virtual HWND GetWindow();
				virtual std::wstring GetValue();
				virtual HWND Create(HWND parent);
				virtual void Changed();
				virtual void Update();

			protected:
				ref<FilePropertyWnd> _pw;
				std::wstring* _path;
				const wchar_t* _filter;
		};
	}
}

#endif