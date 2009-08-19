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
				void AddAction(ref<Runnable> action, bool wait=false);
				void Quit();
				virtual void ShowLogWindow(bool t);

			protected:
				RunnableApplication* _app;
				CriticalSection _actionLock;
				ref<EventLogger> _eventLogger;
				std::vector< ref<Runnable> > _actions;
				Event _actionEvent;
				Event _actionsProcessedEvent;
				graphics::GraphicsInit* _init;

				void ProcessActions();
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