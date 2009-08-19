#include "../include/tjsharedui.h"
using namespace tj::shared;

namespace tj {
	namespace shared {
		class LogThread: public Thread {
			public:
				LogThread();
				virtual ~LogThread();
				virtual void Log(const String& msg);
				virtual void Show(bool s);
				virtual String GetContents();

			protected:
				virtual void Run();

				LoggerWnd* _logger;
				HANDLE _loggerCreatedEvent;
		};
			
		class ThreadedEventLogger: public EventLogger {
			public:
				ThreadedEventLogger();
				virtual ~ThreadedEventLogger();
				virtual void AddEvent(const String& message, ExceptionType e, bool read);
				virtual void ShowLogWindow(bool t);

			protected:
				LogThread _logger;
		};
	}
}

/** LogThread **/
LogThread::LogThread() {
	_loggerCreatedEvent = CreateEvent(NULL, TRUE, FALSE, 0);
	Start();
}

LogThread::~LogThread() {
	PostThreadMessage(_id, WM_QUIT, 0, 0);
	WaitForCompletion();
}

void LogThread::Log(const String& msg) {
	//Start();
	WaitForSingleObject(_loggerCreatedEvent, INFINITE);
	_logger->Log(msg);
}

void LogThread::Show(bool s) {
	_logger->Show(s);
}

String LogThread::GetContents() {
	WaitForSingleObject(_loggerCreatedEvent, INFINITE);
	return _logger->GetContents();
}

void LogThread::Run() {
	try {
		_logger = new LoggerWnd();
		SetEvent(_loggerCreatedEvent);

		MSG msg;
		while(GetMessage(&msg, 0, 0, 0)!=WM_QUIT) {
			try {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			catch(Exception& e) {
				MessageBox(0L, e.GetMsg().c_str(), L"Logger Error", MB_OK|MB_ICONERROR);
			}
		}

		delete _logger;
	}
	catch(Exception& e) {
		MessageBox(0L, e.GetMsg().c_str(), L"Logger Error", MB_OK|MB_ICONERROR);
	}
}

/** ThreadedEventLogger **/
ThreadedEventLogger::ThreadedEventLogger() {
}

ThreadedEventLogger::~ThreadedEventLogger() {
}

void ThreadedEventLogger::AddEvent(const String& message, ExceptionType e, bool read) {
	_logger.Log(message);
}

void ThreadedEventLogger::ShowLogWindow(bool t) {
	_logger.Show(t);
}

/** RunnableApplication **/
RunnableApplication::~RunnableApplication() {
}

void RunnableApplication::AddCommandHistory(ref<Action> action) {
	if(_undo.size()>=KUndoMemory) {
		_undo.pop_front();
	}

	_undo.push_back(action);
}

/** Core **/
Core::Core() {
	_app = 0;
	_init = new graphics::GraphicsInit();
	_eventLogger = GC::Hold(new ThreadedEventLogger());
	Log::SetEventLogger(_eventLogger);
}

void Core::ShowLogWindow(bool t) {
	ref<ThreadedEventLogger> tev = _eventLogger;
	if(tev) {
		tev->ShowLogWindow(t);
	}
}

void Core::Quit() {
	PostQuitMessage(0);
}

void Core::AddAction(ref<Runnable> rm, bool wait) {
	{
		ThreadLock lck(&_actionLock);
		_actions.push_back(rm);
	}
	_actionsProcessedEvent.Reset();
	_actionEvent.Signal();

	if(wait) {
		_actionsProcessedEvent.Wait();
	}
}

void Core::Run(RunnableApplication* app, ref<Arguments> args) {
	_app = app;

    while(true) {
		DWORD result; 
		MSG msg; 

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			// pending queued actions will not be executed after WM_QUIT
			if(msg.message==WM_QUIT) { 
				return;
			}

			_app->Message(msg);
		}

		HANDLE handles[1] = { _actionEvent._event };
		result = MsgWaitForMultipleObjects(1, handles, FALSE, 1000, QS_ALLINPUT|QS_ALLPOSTMESSAGE); 

		// The result tells us the type of event we have.
		if(result==(WAIT_OBJECT_0 + 1)) {
			// New messages have arrived
			continue;
		} 
		else if(result==WAIT_ABANDONED) {
			continue;
		}
		else { 
			// process actions
			ProcessActions();
			_actionsProcessedEvent.Signal();
		}
    }

	_app = 0;
}

void Core::ProcessActions() {
	std::vector< ref<Runnable> > runnables;

	// Copy all current runnables to a separate list so other actions can be added by the runnables
	// (without causing a lockup)
	{
		ThreadLock lock(&_actionLock);
		std::vector< ref<Runnable> >::iterator it = _actions.begin();
		while(it!=_actions.end()) {
			ref<Runnable> rn = *it;
			if(rn) {
				runnables.push_back(rn);
			}
			++it;
		}
		_actions.clear();
		_actionEvent.Reset();
	}
	
	std::vector< ref<Runnable> >::iterator it = runnables.begin();
	while(it!=runnables.end()) {
		ref<Runnable> runnable = *it;
		try {
			runnable->Run();
		}
		catch(Exception& e) {
			if(e.GetType()!=ExceptionTypeMessage) {
				std::wstring msg = e.ToString();
				
				Log::Write(L"TJShow/Core", std::wstring(L"Exception occurred in GUI thread runnable: ")+msg);
			}
		}
		catch(...) {
			Log::Write(L"TJShared/Core", L"Unknown exception occurred in GUI thread runnable");
		}
		++it;
	}
}

RunnableApplication* Core::GetApplicationPointer() {
	return _app;
}

Core::~Core() {
	delete _init;
}

/* ModalLoop */
ModalLoop::ModalLoop(): _running(false), _result(ResultUnknown) {
}

ModalLoop::~ModalLoop() {
}

ModalLoop::Result ModalLoop::Enter(HWND m, bool isDialog) {
	if(!_running) {
		_result = ResultUnknown;
		ReplyMessage(0);
		MSG msg;
		_running = true;

		while(_running && GetMessage(&msg,0,0,0)) {
			TranslateMessage(&msg);

			if(msg.message==WM_KEYDOWN && LOWORD(msg.wParam)==VK_ESCAPE) {
				// End the modal loop whenever the escape key is pressed.
				End(ResultCancelled);
			}
			else if(!isDialog && ((msg.message==WM_KEYUP || msg.message==WM_KEYDOWN) && (msg.wParam==VK_SPACE || msg.wParam==VK_DOWN || msg.wParam==VK_UP))) {
				// Context menus do not take focus (since they do not activate), hence direct all key messages
				// it needs to the window from here.
				msg.hwnd = m;
				DispatchMessage(&msg);
			}
			else if(!isDialog && msg.message==WM_ACTIVATE && msg.wParam==WA_INACTIVE) {
				/* If we're not a dialog (i.e. a context menu), we want to end the modal loop when some other window
				gets activated. Normally, context menu popups shouldn't ever be activated, but under some circumstances,
				they can be. */
				if(!IsChild(m,msg.hwnd)) {
					End(ResultCancelled);
				}
				else {
					DispatchMessage(&msg);
				}
			}
			else if(!isDialog && (msg.message==WM_LBUTTONDOWN || msg.message==WM_RBUTTONDOWN ||msg.message==WM_NCLBUTTONDOWN || msg.message==WM_NCRBUTTONDOWN) && msg.hwnd != m) {
				if(!IsChild(m,msg.hwnd)) {
					End(ResultCancelled);
				}
				else {
					DispatchMessage(&msg);
				}
			}
			else {
				LRESULT ret = DispatchMessage(&msg);

				/* Normally, WM_KEYUP is handled somewhere and the return value is 0. I believe that DefWindowProc
				returns 0 when 'handling' a WM_KEYUP. When we're in a dialog, we want to 'catch' VK_RETURN presses
				so we can end the dialog (emulating a 'default button'). So here, we test the return value of the
				WM_KEYUP message. In normal cases, this will be 0 and we know we can end the dialog. However, multiline
				edit controls really need the VK_RETURN and they will handle it. The EditWndSubClassProc returns a 1 instead
				of 0 when this is the case, so we do not end the dialog. Other windows that really want the VK_RETURN
				should make sure they return anything non-zero from their WM_KEYUP handler whenever VK_RETURN is the
				character code. This probably should be an extra flag in ChildWnd or something, but for the time being,
				this works perfectly... */
				if(isDialog && msg.message==WM_KEYUP && LOWORD(msg.wParam)==VK_RETURN && ret==0) {
					End(ResultSucceeded);
				}
			}

			if(!_running) break;
		}

		return _result;
	}

	return ResultUnknown;
}

void ModalLoop::End(Result r) {
	if(_running) {
		_result = r;
		_running = false;
	}
}

/* Alert */
#ifdef _WIN32
int ConvertAlertType(Alert::AlertType t) {
	switch(t) {
		case Alert::TypeError:
			return MB_ICONERROR;

		case Alert::TypeQuestion:
			return MB_ICONQUESTION;

		case Alert::TypeWarning:
			return MB_ICONWARNING;

		case Alert::TypeInformation:
			return MB_ICONINFORMATION;

		default:
			return 0;
	}
}

void Alert::Show(const std::wstring& title, const std::wstring& text, AlertType t) {
	HWND parent = 0L;
	MessageBox(parent, text.c_str(), title.c_str(), MB_OK|ConvertAlertType(t));
}

bool Alert::ShowOKCancel(const std::wstring& title, const std::wstring& text, AlertType t) {
	HWND parent = 0L;
	return MessageBox(parent, text.c_str(), title.c_str(), MB_OKCANCEL|ConvertAlertType(t)) == IDOK;
}

bool Alert::ShowYesNo(const std::wstring& title, const std::wstring& text, AlertType t, bool modal) {
	return MessageBox(0L, text.c_str(), title.c_str(), MB_YESNO|ConvertAlertType(t)|(modal?MB_TASKMODAL|MB_SETFOREGROUND:0)) == IDYES;
}

Alert::Result Alert::ShowYesNoCancel(const std::wstring& title, const std::wstring& text, AlertType t, bool modal) {
	int r = MessageBox(0L, text.c_str(), title.c_str(), MB_YESNOCANCEL|ConvertAlertType(t)|(modal?MB_TASKMODAL|MB_SETFOREGROUND:0));

	switch(r) {
		case IDYES:
			return ResultYes;

		case IDNO:
			return ResultNo;

		case IDCANCEL:
		default:
			return ResultCancel;
	}
}
#endif>>>>>>> .r487
