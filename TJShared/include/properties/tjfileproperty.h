#ifndef _TJFILEPROPERTY_H
#define _TJFILEPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED FilePropertyWnd: public ChildWnd {
			public:
				FilePropertyWnd(const std::wstring& name, ResourceIdentifier* rid, strong<ResourceProvider> rmg, const wchar_t* filter);
				virtual ~FilePropertyWnd();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void OnDropFiles(const std::vector< std::wstring >& files);
				virtual void Update();
				virtual void OnSize(const Area& ns);
				virtual void Layout();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);

			protected:
				void SetFile(const std::wstring& file);
				virtual void OnMouse(MouseEvent me, Pixels x, Pixels y);
				ResourceIdentifier* _path;
				std::wstring _name;
				Icon _linkIcon;
				const wchar_t* _filter;
				strong<ResourceProvider> _rmg;
				ref<EditWnd> _edit;
		};

		class EXPORTED FileProperty: public Property {
			public:
				FileProperty(const std::wstring& name, ResourceIdentifier* path, strong<ResourceProvider> rmg, const wchar_t* filter=L"*.*\0\0\0");
				virtual ~FileProperty();
				virtual ref<Wnd> GetWindow();
				virtual void Update();

			protected:
				ref<FilePropertyWnd> _pw;
				strong<ResourceProvider> _rmg;
				ResourceIdentifier* _path;
				const wchar_t* _filter;
		};
	}
}

#endif