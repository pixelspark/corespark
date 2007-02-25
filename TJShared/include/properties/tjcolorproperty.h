#ifndef _TJCOLORPROPERTY_H
#define _TJCOLORPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED ColorChooserWnd: public ChildWnd {
			public:
				ColorChooserWnd(HWND parent, unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char* tred, unsigned char* tgreen, unsigned char* tblue);
				virtual ~ColorChooserWnd();
				virtual void Paint(Gdiplus::Graphics& g);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);

			protected:
				unsigned char* _red;
				unsigned char* _green;
				unsigned char* _blue;
				unsigned char* _tred;
				unsigned char* _tgreen;
				unsigned char* _tblue;
		};

		class EXPORTED ColorProperty: public Property {
			public:
				ColorProperty(std::wstring name,unsigned char* red, unsigned char* green, unsigned char* blue,unsigned char* tred, unsigned char* tgreen, unsigned char* tblue);
				virtual ~ColorProperty();
				virtual HWND GetWindow();
				virtual std::wstring GetValue();
				virtual HWND Create(HWND parent);
				virtual void Changed();
				virtual void Update();

			protected:
				unsigned char* _red;
				unsigned char* _green;
				unsigned char* _blue;
				unsigned char* _tRed;
				unsigned char* _tBlue;
				unsigned char* _tGreen;
				ref<ColorChooserWnd> _wnd;
		};
	}
}
#endif