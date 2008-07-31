#ifndef _TJTEXTPROPERTY_H
#define _TJTEXTPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED TextProperty: public GenericProperty<std::wstring> {
			public:
				TextProperty(const std::wstring& name, std::wstring* value, Pixels height = 100);
				virtual ~TextProperty();
				virtual Pixels GetHeight();

			protected:
				Pixels _height;
		};
	}
}

#endif