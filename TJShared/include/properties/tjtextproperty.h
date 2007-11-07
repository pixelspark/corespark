#ifndef _TJTEXTPROPERTY_H
#define _TJTEXTPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED TextProperty: public virtual Object, public Property {
			public:
				TextProperty(std::wstring name, std::wstring* value, Pixels height = 100);
				virtual ~TextProperty();
				virtual std::wstring GetValue();
				virtual HWND GetWindow();
				virtual HWND Create(HWND parent);
				
				// Called when the value in the edit window has changed (EN_CHANGED)
				virtual void Changed();
				
				// Called when a repaint is about to begin and the value in the window needs to be updated
				virtual void Update();
				virtual Pixels GetHeight();

			protected:
				std::wstring* _value;
				HWND _wnd;
				Pixels _height;
		};
	}
}

#endif