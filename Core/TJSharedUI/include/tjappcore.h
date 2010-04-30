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
 
 #ifndef _TJAPPCORE_H
#define _TJAPPCORE_H

namespace tj {
	namespace shared {	
		namespace graphics {
			class GraphicsInit;
		}

		class EXPORTED RunnableApplication {
			public:
				virtual ~RunnableApplication();
			
				#ifdef TJ_OS_WIN
					virtual void Message(MSG& msg) = 0;
				#endif
			
				virtual void AddCommandHistory(ref<Action> action);

				static const int KUndoMemory = 10;
				std::deque< ref<Action> > _undo;
		};

		class EXPORTED Core: public Singleton<Core> {
			public:
				void Run(RunnableApplication* app, ref<Arguments> args);
				RunnableApplication* GetApplicationPointer();
				Core();
				virtual ~Core();
				void Quit();
				virtual void ShowLogWindow(bool t);

			protected:
				RunnableApplication* _app;
				ref<EventLogger> _eventLogger;
				graphics::GraphicsInit* _init;
		};

		class EXPORTED ModalLoop {
			public:
				ModalLoop();
				virtual ~ModalLoop();
				
				enum Result {
					ResultUnknown = 0,
					ResultSucceeded = 1,
					ResultCancelled = 2,
				};
				
				#ifdef TJ_OS_WIN
					virtual Result Enter(HWND m, bool isDialog);
				#endif
			
				virtual void End(Result r = ResultUnknown);

			protected:
				volatile bool _running;
				volatile Result _result;
		};

		class EXPORTED Alert {
			public:
				enum AlertType {
					TypeNone = 0,
					TypeInformation,
					TypeWarning,
					TypeError,
					TypeQuestion,
				};

				enum Result {
					ResultYes = 1,
					ResultNo = 0,
					ResultCancel = -1,
				};

				static void Show(const String& title, const String& text, AlertType t);
				static bool ShowOKCancel(const String& title, const String& text, AlertType t);
				static bool ShowYesNo(const String& title, const String& text, AlertType t, bool modal = false);
				static Result ShowYesNoCancel(const String& title, const String& text, AlertType t, bool modal = false);
		};
	}
}

#endif