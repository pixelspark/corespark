#ifndef _TJREADONLYPROPERTY_H
#define _TJREADONLYPROPERTY_H

namespace tj {
	namespace shared {
		template<typename T> class GenericReadOnlyProperty: public GenericProperty<T> {
			public:
				GenericReadOnlyProperty(const std::wstring& name, T* value): GenericProperty<T>(name,value,0, *value) {
				}

				virtual ~GenericReadOnlyProperty() {
				}

				virtual ref<Wnd> GetWindow() {
					/* If the window in the underlying property is an EditWnd or CheckboxWnd, we can
					 make it read-only. TODO: Create a method GenericProperty<T>::SetReadOnly(...) to do this. */
					ref<Wnd> w = GenericProperty<T>::GetWindow();
					if(w) {
						if(w.IsCastableTo<EditWnd>()) {
							ref<EditWnd> ew = w;
							if(ew) {
								ew->SetReadOnly(true);
							}
						}
						else if(w.IsCastableTo<CheckboxWnd>()) {
							ref<CheckboxWnd> ew = w;
							if(ew) {
								ew->SetReadOnly(true);
							}
						}
					}
					return w;
				}
		};
	}
}

#endif