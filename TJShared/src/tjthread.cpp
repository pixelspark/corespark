#include "../include/tjcore.h"
using namespace tj::shared;

volatile long Thread::_count = 0;

// SetThreadName, see ms-help://MS.MSDNQTR.v80.en/MS.MSDN.v80/MS.VisualStudio.v80.en/dv_vsdebug/html/c85d0968-9f22-4d69-87f4-acca2ae777b8.htm
#define MS_VC_EXCEPTION 0x406D1388

typedef struct tagTHREADNAME_INFO {
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName) {
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = szThreadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try {
      RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
   }
   __except(EXCEPTION_CONTINUE_EXECUTION) {
   }
}

namespace tj {
	namespace shared {
		DWORD WINAPI ThreadProc(LPVOID lpParam) {
			try {
				InterlockedIncrement(&Thread::_count);
				Thread* tr = (Thread*)lpParam;
				if(tr!=0) {
					srand(GetTickCount());
					tr->Run();
				}
			}
			catch(...) {
			}
			
			InterlockedDecrement(&Thread::_count);
			return 0;
		}
	}
}


/* Thread */
Thread::Thread() {
	_thread = CreateThread(NULL, 4096, ThreadProc, (LPVOID)this, CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION, (LPDWORD)&_id);
	_started = false;
}

Thread::~Thread() {
	CloseHandle(_thread);
}

long Thread::GetThreadCount() {
	return _count;
}

void Thread::SetName(const char* t) {
	SetThreadName(_id, t);
}

void Thread::Start() {
	DWORD code = -1;
	GetExitCodeThread(_thread, &code);

	if(code==STILL_ACTIVE) {
		// we're running...
		_started = true;
		ResumeThread(_thread);
	}
	else {
		// finished correctly, run again
		CloseHandle(_thread);
		_thread = CreateThread(NULL, 512, ThreadProc, (LPVOID)this, CREATE_SUSPENDED, (LPDWORD)&_id);
		_started = true;
		ResumeThread(_thread);
	}
}

void Thread::WaitForCompletion() {
	if(!_started) {
		// don't wait if the thread was never started
		return;
	}
	
	if(GetCurrentThread()==_thread) {
		return; // Cannot wait on yourself
	}

	WaitForSingleObject(_thread,INFINITE);
}

void Thread::Terminate() {
	TerminateThread(_thread, 0);
}

int Thread::GetID() const {
	return _id;
}

void Thread::Run() {
}

/* Semaphore; Windows implementation */
#ifdef _WIN32
	Semaphore::Semaphore() {
		_sema = CreateSemaphore(NULL, 0, LONG_MAX, 0);
	}

	Semaphore::~Semaphore() {
		CloseHandle(_sema);
	}

	void Semaphore::Release(int n) {
		ReleaseSemaphore(_sema, n, NULL);
	}

	bool Semaphore::Wait() {
		return WaitForSingleObject(_sema, INFINITE) == WAIT_OBJECT_0;
	}

	HANDLE Semaphore::GetHandle() {
		return _sema;
	}
#endif

/* Event; Windows implementation */
#ifdef _WIN32
	Event::Event() {
		_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	Event::~Event() {
		CloseHandle(_event);
	}

	void Event::Signal() {
		SetEvent(_event);
	}

	void Event::Pulse() {
		PulseEvent(_event);
	}

	void Event::Reset() {
		ResetEvent(_event);
	}

	HANDLE Event::GetHandle() {
		return _event;
	}

	void Event::Wait(int ms) {
		WaitForSingleObject(_event, ms);
	}
#endif

/* ThreadLocal; Windows implementation uses TLS */
#ifdef _WIN32
	ThreadLocal::ThreadLocal() {
		_tls = TlsAlloc();
		if(_tls==TLS_OUT_OF_INDEXES) {
			Throw(L"Cannot create thread-local storage", ExceptionTypeSevere);
		}
	}

	ThreadLocal::~ThreadLocal() {
		TlsFree(_tls);
	}

	int ThreadLocal::GetValue() const {
		return (int)(__int64)TlsGetValue(_tls);
	}

	void ThreadLocal::SetValue(int v) {
		TlsSetValue(_tls, (void*)(__int64)v);
	}
#endif

ThreadLocal::operator int() const {
	return GetValue();
}

void ThreadLocal::operator=(int r) {
	SetValue(r);
}
