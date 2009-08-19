#ifndef _TJFILEPROPERTY_H
#define _TJFILEPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED FilePropertyWnd: public ChildWnd {
			public:
				FilePropertyWnd(const String& name, ref<Inspectable> holder, ResourceIdentifier* rid, strong<ResourceProvider> rmg, const wchar_t* filter);
				virtual ~FilePropertyWnd();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void OnDropFiles(const std::vector< String >& files);
				virtual void Update();
				virtual void OnSize(const Area& ns);
				virtual void Layout();
			
				#ifdef TJ_OS_WIN
					virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				#else
					#warning Needs a Message implementation on non-Windows
				#endif

			protected:
				void SetFile(const String& file);
				virtual void OnMouse(MouseEvent me, Pixels x, Pixels y);
				ResourceIdentifier* _path;
				String _name;
				Icon _linkIcon;
				const wchar_t* _filter;
				strong<ResourceProvider> _rmg;
				ref<EditWnd> _edit;
				weak<Inspectable> _holder;
		};

		class EXPORTED FileProperty: public Property {
			public:
				FileProperty(const String& name, ref<Inspectable> holder, ResourceIdentifier* path, strong<ResourceProvider> rmg, const wchar_t* filter=L"*.*\0\0\0");
				virtual ~FileProperty();
				virtual ref<Wnd> GetWindow();
				virtual void Update();

			protected:
				weak<Inspectable> _holder;
				ref<FilePropertyWnd> _pw;
				strong<ResourceProvider> _rmg;
				ResourceIdentifier* _path;
				const wchar_t* _filter;
		};
	}
}

#endif