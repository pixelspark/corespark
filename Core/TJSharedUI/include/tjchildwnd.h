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
 
 #ifndef _TJCHILDWND_H
#define _TJCHILDWND_H

namespace tj {
	namespace shared {
		class EXPORTED ChildWnd: public Wnd {
			public:
				ChildWnd(bool useDoubleBuffering = true);
				virtual void SetTabStop(bool ts);
				virtual bool GetTabStop();
		};

		class EXPORTED CheckboxWnd: public ChildWnd {
			public:
				CheckboxWnd();
				virtual ~CheckboxWnd();
				virtual bool IsChecked() const;
				virtual void SetChecked(bool t);
				virtual void SetReadOnly(bool r);
				virtual bool IsReadOnly() const;
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);

			protected:
				virtual void OnTimer(unsigned int id);
				virtual void OnSize(const Area& ns);
				virtual void OnFocus(bool f);
				virtual void OnKey(Key k, wchar_t ch, bool down, bool isAccelerator);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);

				Icon _checkedIcon;
				Animation _checkAnimation;

			private:
				bool _checked;
				bool _readOnly;
		};
	}
}

#endif