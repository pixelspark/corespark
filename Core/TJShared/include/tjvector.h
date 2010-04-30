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
 
 #ifndef _TJVECTOR_H
#define _TJVECTOR_H

#include "tjsharedinternal.h"
#include "tjserializable.h"

namespace tj {
	namespace shared {
		class EXPORTED Vector: public Serializable {
			public:
				Vector(float fx, float fy, float fz);
				Vector(const Vector& other);
				~Vector();
				Vector Dot(const Vector& other) const;
				Vector Scale(float n) const;
				float GetLength() const;
				String ToString() const;

				virtual void Save(TiXmlElement* parent);
				virtual void Load(TiXmlElement* you);

				inline Vector operator+(const Vector& o) {
					return Vector(x+o.x, y+o.y, z+o.z);
				}

			public:
				float x,y,z;
		};
	}
}

#endif