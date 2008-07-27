#ifndef _TJFILEPROPERTY_H
#define _TJFILEPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED FilePropertyWnd: public ChildWnd {
			public:
				FilePropertyWnd(std::wstring name, std::wstring* path, const wchar_t* filter);
				virtual ~FilePropertyWnd();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void OnDropFiles(const std::vector< std::wstring >& files);
				virtual void Update();
				virtual void OnSize(const Area& ns);
				virtual void Layout();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);

			protected:
				void SetFile(const std::wstring& file);
				virtual void OnMouse(MouseEvent me, Pixels x, Pixels y);
				std::wstring* _path;
				std::wstring _name;
				Icon _linkIcon;
				const wchar_t* _filter;
				ref<EditWnd> _edit;
		};

		class EXPORTED FileProperty: public Property {
			public:
				FileProperty(const std::wstring& name, std::wstring* path, const wchar_t* filter=L"*.*\0\0\0");
				virtual ~FileProperty();
				virtual ref<Wnd> GetWindow();
				virtual void Update();

			protected:
				ref<FilePropertyWnd> _pw;
				std::wstring* _path;
				const wchar_t* _filter;
		};
	}
}

#endif