#ifndef _TJVECTORPROPERTY_H
#define _TJVECTORPROPERTY_H

namespace tj {
	namespace shared {
		class VectorPropertyWnd;

		class EXPORTED VectorProperty: public Property {
			public:
				VectorProperty(const std::wstring& name, Vector* v);
				virtual ~VectorProperty();
				virtual ref<Wnd> GetWindow();
				virtual void Update();

			protected:
				Vector* _vec;
				VectorPropertyWnd* _wnd;
		};
	}
}

#endif