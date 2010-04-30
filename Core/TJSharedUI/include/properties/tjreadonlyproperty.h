/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJREADONLYPROPERTY_H
#define _TJREADONLYPROPERTY_H

namespace tj {
	namespace shared {
		template<typename T> class GenericReadOnlyProperty: public GenericProperty<T> {
			public:
				GenericReadOnlyProperty(const String& name, ref<Inspectable> holder, T* value): GenericProperty<T>(name, holder, value, *value) {
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