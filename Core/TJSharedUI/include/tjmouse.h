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
 
 #ifndef _TJMOUSE_H
#define _TJMOUSE_H

namespace tj {
	namespace shared {
		class Mouse;
		class Wnd;

		enum Cursor {
			CursorDefault = 0,
			CursorHand,
			CursorHandGrab,
			CursorBusy,
			CursorSizeNorthSouth,
			CursorSizeEastWest,
			CursorSizeAll,
			CursorBeam,
		};

		class EXPORTED MouseCapture {
			public:
				MouseCapture();
				~MouseCapture();
				void StartCapturing(strong<Mouse> mouse, strong<Wnd> wnd);
				void StopCapturing();

			private:
				bool _isCapturing;
		};

		class EXPORTED Mouse: public virtual Object {
			friend class MouseCapture;

			public:
				enum Button {
					ButtonLeft = 0,
					ButtonRight,
					ButtonMiddle,
				};

				static strong<Mouse> Instance();
				virtual ~Mouse();
				virtual void SetCursorType(const Cursor& c) = 0;
				virtual void SetCursorHidden(bool t) = 0;
				virtual bool IsCursorHidden() const = 0;
				virtual bool IsButtonDown(Button b) = 0;
				virtual Coord GetCursorPosition(ref<Wnd> w) = 0;

			protected:
				static ref<Mouse> _mouse;
		};
	}
}

#endif