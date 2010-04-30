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
 
 #ifndef _TJLOGGER_H
#define _TJLOGGER_H

namespace tj {
	namespace shared {
		class EXPORTED LoggerWnd: public Wnd {
			public:
				LoggerWnd();
				virtual ~LoggerWnd();
				void Log(const String& ws);
				virtual void Layout();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual String GetContents();

			protected:
				#ifdef TJ_OS_WIN
					virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
			
					HFONT _font;
					HWND _list;
				#endif
		};
	}
}

#endif