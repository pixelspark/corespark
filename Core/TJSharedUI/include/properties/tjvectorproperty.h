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