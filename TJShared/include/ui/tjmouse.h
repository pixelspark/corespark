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
				static strong<Mouse> Instance();
				virtual ~Mouse();
				virtual void SetCursorType(const Cursor& c) = 0;
				virtual void SetCursorHidden(bool t) = 0;
				virtual bool IsCursorHidden() const = 0;

			protected:
				static ref<Mouse> _mouse;
		};
	}
}

#endif