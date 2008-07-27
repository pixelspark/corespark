#include "../../include/ui/tjui.h"
using namespace tj::shared;

ref<Core> Core::_instance;

RunnableApplication::~RunnableApplication() {
}

void RunnableApplication::AddCommandHistory(ref<Action> action) {
	if(_undo.size()>=KUndoMemory) {
		_undo.pop_front();
	}

	_undo.push_back(action);
}

Runnable::~Runnable() {
}

strong<Core> Core::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new Core());
	}

	return _instance;
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

		HANDLE handles[1] = { _actionEvent.GetHandle() };
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

Core::Core() {
	_app = 0;
	_init = new GraphicsInit();
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

		while(GetMessage(&msg,0,0,0) && _running) {
			TranslateMessage(&msg);

			if(msg.message==WM_KEYDOWN && LOWORD(msg.wParam)==VK_ESCAPE) {
				// End modal loop
				End(ResultCancelled);
			}
			else if(!isDialog && ((msg.message==WM_KEYUP || msg.message==WM_KEYDOWN) && (msg.wParam==VK_SPACE || msg.wParam==VK_DOWN || msg.wParam==VK_UP))) {
				// Context menus do not take focus (since they do not activate), hence direct all key messages
				// it needs to the window from here.
				msg.hwnd = m;
				DispatchMessage(&msg);
			}
			else if(!isDialog && msg.message==WM_ACTIVATE && msg.wParam==WA_INACTIVE) {
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
				DispatchMessage(&msg);
			}
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
	MessageBox(0L, text.c_str(), title.c_str(), MB_OK|ConvertAlertType(t));
}

bool Alert::ShowOKCancel(const std::wstring& title, const std::wstring& text, AlertType t) {
	return MessageBox(0L, text.c_str(), title.c_str(), MB_OKCANCEL|ConvertAlertType(t)) == IDOK;
}

bool Alert::ShowYesNo(const std::wstring& title, const std::wstring& text, AlertType t) {
	return MessageBox(0L, text.c_str(), title.c_str(), MB_YESNO|ConvertAlertType(t)) == IDYES;
}

Alert::Result Alert::ShowYesNoCancel(const std::wstring& title, const std::wstring& text, AlertType t) {
	int r = MessageBox(0L, text.c_str(), title.c_str(), MB_YESNO|ConvertAlertType(t));
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
#endif