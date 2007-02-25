#ifndef _TJVECTORPROPERTY_H
#define _TJVECTORPROPERTY_H

namespace tj {
	namespace shared {
		class VectorPropertyWnd;

		class EXPORTED VectorProperty: public Property {
			public:
				VectorProperty(std::wstring name, Vector* v);
				virtual ~VectorProperty();
				virtual std::wstring GetValue();

				virtual HWND GetWindow();
				virtual HWND Create(HWND parent);
				virtual void Changed();
				virtual void Update();

			protected:
				Vector* _vec;
				VectorPropertyWnd* _wnd;
		};
	}
}

#endif