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
				virtual void GetAccelerators(std::vector<Accelerator>& alist);

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
				MouseCapture _capture;

				const static int KMinimumWidth = 30;
				const static int KValueHeight = 25;
		};
	}
}

#endif
