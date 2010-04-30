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
 
 #ifndef _TJICON_H
#define _TJICON_H

#include "internal/tjpch.h"

namespace tj {
	namespace shared {
		class EXPORTED Icon {
			public:
				Icon(const String& rid);
				virtual ~Icon();
				Icon(graphics::Image* bmp); // Icon will own and delete bmp!
				operator graphics::Image*();
				graphics::Image* GetBitmap();
				bool IsLoaded() const;
				void Paint(graphics::Graphics& g, const Area& rc, bool enabled = true);
				void Paint(graphics::Graphics& g, const Area& rc, float alpha);
				void Paint(graphics::Graphics& g, const Area& rc, bool enabled, float alpha);

			protected:
				graphics::Image* _bitmap;
		};
	}
}

#endif