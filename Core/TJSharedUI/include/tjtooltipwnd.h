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
 
 #ifndef _TJTOOLTIP_H
#define _TJTOOLTIP_H

namespace tj {
	namespace shared {
		class EXPORTED TooltipWnd: public virtual Object {
			public:
				// TODO change this to TooltipWnd(ref<Wnd> parent);
				#ifdef TJ_OS_WIN
					TooltipWnd(HWND parent);
				#endif
			
				virtual ~TooltipWnd();
				virtual void SetTrackEnabled(bool t);
				virtual void SetTrackPosition(int x, int y);
				virtual void Move(Pixels x, Pixels y);
				virtual void SetTooltip(String text);

			protected:
				#ifdef TJ_OS_WIN
					HWND _owner;
					HWND _wnd;
				#endif
		};
	}
}

#endif