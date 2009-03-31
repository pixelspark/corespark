#ifndef _TJSLIDER_H
#define _TJSLIDER_H

namespace tj {
	namespace shared {
		class EXPORTED SliderWnd: public ChildWnd {
			public:
				SliderWnd(const String& title = L"");
				virtual ~SliderWnd();
				float GetValue() const;
				void SetValue(float f, bool notify=true);
				void SetDisplayValue(float f, bool notify);
				float GetDisplayValue() const;
				void SetMarkValue(float f);
				float GetMarkValue() const;
				virtual void Update();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void SetColor(int idx);
				void SetShowValue(bool show);
				void SetSnapToHalf(bool snap);
				virtual void Fill(LayoutFlags flags, Area& rect, bool direct = true);

				struct NotificationUpdate {
				};

				struct NotificationChanged {
				};

				Listenable<NotificationUpdate> EventUpdate;
				Listenable<NotificationChanged> EventChanged;

			protected:
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnMouseWheelMove(WheelDirection wd);
				virtual void OnFocus(bool focus);
				virtual void OnKey(Key k, wchar_t ch, bool down, bool accel);
				virtual void OnSize(const Area& ns);

				float _value;
				float _displayValue;
				int _color;
				float _mark;
				bool _showValue;
				bool _snapHalf;
				Pixels _startX, _startY;
				bool _preciseDrag;

				const static int KMinimumWidth = 30;
				const static int KValueHeight = 25;
		};
	}
}

#endif
