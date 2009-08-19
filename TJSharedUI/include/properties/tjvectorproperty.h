#ifndef _TJVECTORPROPERTY_H
#define _TJVECTORPROPERTY_H

namespace tj {
	namespace shared {
		class VectorPropertyWnd;

		class EXPORTED VectorProperty: public Property {
			public:
				VectorProperty(const String& name, Vector* v);
				virtual ~VectorProperty();
				virtual ref<Wnd> GetWindow();
				virtual void Update();
				virtual void SetDimensionShown(unsigned int d, bool h);

			protected:
				Vector* _vec;
				ref<Wnd> _wnd;
		};
	}
}

#endif