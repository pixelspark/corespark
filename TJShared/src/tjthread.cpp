#include "../include/tjshared.h"
using namespace tj::shared;

#ifdef TJ_OS_POSIX
	#define TJ_USE_PTHREADS
	#include <pthread.h>
	#include <sys/time.h>
#endif

#ifdef TJ_OS_MAC
	#include <dispatch/dispatch.h>
#endif

volatile ReferenceCount Thread::_count = 0;

std::map<int, String> Thread::_names;
CriticalSection Thread::_nameLock;

namespace tj {
	namespace shared {
		#ifdef TJ_OS_WIN
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
		#endif
		
		#ifdef TJ_USE_PTHREADS
			void* ThreadProc(void* arg) {
				try {
					#ifdef TJ_OS_MAC
						OSAtomicAdd32(1, &Thread::_count);
					#endif
					
					Thread* tr = (Thread*)arg;
					if(tr!=0) {
						srand(time(NULL));
						tr->Run();
					}
				}
				catch(...) {
				}
				
				#ifdef TJ_OS_MAC
					OSAtomicAdd32(-1, &Thread::_count);
				#endif
				return NULL;
			}
		#endif
	}
}



/* Thread */
Thread::Thread() {
	#ifdef TJ_OS_WIN
		_thread = CreateThread(NULL, 4096, ThreadProc, (LPVOID)this, CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION, (LPDWORD)&_id);
	#endif
	
	#ifdef TJ_USE_PTHREADS
		_thread = 0;
	#endif
	
	_started = false;
}

Thread::~Thread() {
	#ifdef TJ_OS_WIN
		CloseHandle(_thread);
	#endif
	
	#ifdef TJ_USE_PTHREADS
		if(_thread!=0) {
			pthread_detach(_thread);
		}
	#endif
}

long Thread::GetThreadCount() {
	return _count;
}

void Thread::SetName(const String& t) {
	ThreadLock lock(&_nameLock);
	_names[_id] = t;
}

int Thread::GetCurrentThreadID() {
	#ifdef TJ_OS_WIN
		return (unsigned int)(::GetCurrentThreadId());
	#endif
	
	#ifdef TJ_USE_PTHREADS
		#ifdef TJ_OS_LINUX
			return (int)static_cast<long long>(pthread_self());
		#endif
	
		#ifdef TJ_OS_MAC
			return (int)reinterpret_cast<long long> (pthread_self());
		#endif
	#endif
}

String Thread::GetCurrentThreadName() {
	int tid = GetCurrentThreadID();

	ThreadLock lock(&_nameLock);
	std::map<int, String>::const_iterator it = _names.find(tid);
	if(it!=_names.end()) {
		return it->second;
	}
	return L"";
}

void Thread::SetPriority(Priority p) {
	#ifdef TJ_OS_WIN
		// Convert priority to Win32 priority code
		int prio = THREAD_PRIORITY_NORMAL;
		switch(p) {
			case PriorityAboveNormal:
			case PriorityHigh:
				prio = THREAD_PRIORITY_ABOVE_NORMAL;
				break;

			case PriorityIdle:
				prio = THREAD_PRIORITY_IDLE;
				break;

			case PriorityBelowNormal:
			case PriorityLow:
				prio = THREAD_PRIORITY_BELOW_NORMAL;
				break;

			case PriorityTimeCritical:
				prio = THREAD_PRIORITY_TIME_CRITICAL;
				break;

			default:
			case PriorityNormal:
				prio = THREAD_PRIORITY_NORMAL;
		}

		SetThreadPriority(_thread, prio);
	#endif
	
	#ifdef TJ_USE_PTHREADS
		sched_param sp;
		int sched_policy = 0;
		if(pthread_getschedparam(_thread, &sched_policy, &sp)==0) {
			int minPriority = sched_get_priority_min(sched_policy);
			int maxPriority = sched_get_priority_max(sched_policy);
			int priorityQuantum = (maxPriority-minPriority)/6;
			int normalPriority = (maxPriority-minPriority)/2;
			
			switch(p) {
				case PriorityAboveNormal:
				case PriorityHigh:
					sp.sched_priority = normalPriority+priorityQuantum;
					break;
					
				case PriorityIdle:
					sp.sched_priority = minPriority;
					break;
					
				case PriorityBelowNormal:
				case PriorityLow:
					sp.sched_priority = normalPriority-priorityQuantum;
					break;
					
				case PriorityTimeCritical:
					sp.sched_priority = maxPriority;
					break;
					
				default:
				case PriorityNormal:
					sp.sched_priority = normalPriority;
			}
			pthread_setschedparam(_thread, sched_policy, &sp);
		}
	#endif
}

void Thread::Start() {
	#ifdef TJ_OS_WIN
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
	#endif
	
	#ifdef TJ_USE_PTHREADS
		if(_thread==0) {
			pthread_attr_t  attr;
			if(pthread_attr_init(&attr)) {
				Throw(L"Could not initialize pthread attribute structure", ExceptionTypeError);
			}
			
			if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) {
				Throw(L"Could not set detach state in thread attribute structure", ExceptionTypeError);
			}
			
			if(pthread_create(&_thread, &attr, ThreadProc, reinterpret_cast<void*>(this))) {
				Throw(L"pthread_create failed", ExceptionTypeError);
			}
			_started = true;
		}
	#endif
}

void Thread::WaitForCompletion() {
	if(!_started) {
		// don't wait if the thread was never started
		return;
	}
	
	#ifdef TJ_OS_WIN
		if(GetCurrentThread()==_thread) {
			return; // Cannot wait on yourself
		}
	
		WaitForSingleObject(_thread,INFINITE);
	#endif
	
	#ifdef TJ_USE_PTHREADS
		if(pthread_self()==_thread) {
			return; // Cannot wait on yourself
		}
	
		void* returnValue = 0L;
		int r = pthread_join(_thread, &returnValue);
		if(r!=0) {
			std::wostringstream wos;
			wos << L"pthread_join failed: " << r;
			std::wstring emsg = wos.str();
			Throw(emsg.c_str(), ExceptionTypeError);
		}
	#endif	
}

void Thread::Terminate() {
	#ifdef TJ_OS_WIN
		TerminateThread(_thread, 0);
	#endif
	
	#ifdef TJ_OS_MAC
		pthread_kill(reinterpret_cast<pthread_t>(_thread), SIGKILL);
	#endif
}

int Thread::GetID() const {
	return _id;
}

void Thread::Run() {
}

void Thread::Sleep(double ms) {
	#ifdef TJ_OS_WIN
	::Sleep(DWORD(ms));
	#endif

	#ifdef TJ_OS_MAC
		usleep(ms*1000.0);
	#endif
}

/* Semaphore; Windows implementation */
#ifdef TJ_OS_WIN
	Semaphore::Semaphore() {
		_sema = CreateSemaphore(NULL, 0, LONG_MAX, 0);
	}

	Semaphore::~Semaphore() {
		CloseHandle(_sema);
	}

	void Semaphore::Release(int n) {
		ReleaseSemaphore(_sema, n, NULL);
	}

	bool Semaphore::Wait(const Time& out) {
		int timeoutMS = out.ToInt();
		return WaitForSingleObject(_sema, (timeoutMS>0)?timeoutMS:INFINITE) == WAIT_OBJECT_0;
	}

	HANDLE Semaphore::GetHandle() {
		return _sema;
	}
#endif

#ifdef TJ_OS_MAC
	Semaphore::Semaphore() {
		_sema = reinterpret_cast<void*>(dispatch_semaphore_create(0));
	}

	Semaphore::~Semaphore() {
		dispatch_release(reinterpret_cast<dispatch_semaphore_t>(_sema));
	}

	void Semaphore::Release(int n) {
		while(n>0) {
			dispatch_semaphore_signal(reinterpret_cast<dispatch_semaphore_t>(_sema));
			--n;
		}
	}

	bool Semaphore::Wait(const Time& out) {
		int timeoutMS = out.ToInt();
		return dispatch_semaphore_wait(reinterpret_cast<dispatch_semaphore_t>(_sema), (timeoutMS<0)?DISPATCH_TIME_FOREVER:dispatch_time(DISPATCH_TIME_NOW, timeoutMS*1000))==0;
	}
#endif

/* Event; Windows implementation */
#ifdef TJ_OS_WIN
	Event::Event() {
		_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(_event==NULL) {
			Throw(L"Could not create Event!", ExceptionTypeError);
		}
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

	bool Event::Wait(int ms) {
		return WaitForSingleObject(_event, (ms<=0) ? INFINITE : ms)==WAIT_OBJECT_0;
	}

	HANDLE Event::GetHandle() {
		return _event;
	}
#endif

#ifdef TJ_USE_PTHREADS
Event::Event() {
	pthread_cond_init(&_event, NULL);
}

Event::~Event() {
	pthread_cond_destroy(&_event);
}

void Event::Signal() {
	pthread_cond_signal(&_event);
}

void Event::Pulse() {
	pthread_cond_signal(&_event);
}

void Event::Reset() {
	// TODO: not necessary?
}

bool Event::Wait(int ms) {
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	
	if(ms==0) {
		pthread_cond_wait(&_event, &lock);
		return true;
	}
	else {
		struct timeval now;
		gettimeofday(&now, NULL);
		
		struct timespec abstime;
		abstime.tv_sec = now.tv_sec + (ms / 1000);
		abstime.tv_nsec = (now.tv_usec + ((ms % 1000)*1000));
		return pthread_cond_timedwait(&_event, &lock, &abstime)==0; /* When timing out, pthread_cond_timedwait returns ETIMEOUT */
	}
}
#endif

/* ThreadLocal; Windows implementation uses TLS */
#ifdef TJ_OS_WIN
	ThreadLocal::ThreadLocal() {
		_tls = TlsAlloc();
		if(_tls==TLS_OUT_OF_INDEXES) {
			Throw(L"Cannot create thread-local storage", ExceptionTypeSevere);
		}
	}

	ThreadLocal::~ThreadLocal() {
		TlsFree(_tls);
	}

	void* ThreadLocal::GetValue() const {
		return TlsGetValue(_tls);
	}

	void ThreadLocal::SetValue(void* v) {
		TlsSetValue(_tls, v);
	}
#endif

#ifdef TJ_USE_PTHREADS
	ThreadLocal::ThreadLocal() {
		pthread_key_create(&_tls, NULL);
	}

	ThreadLocal::~ThreadLocal() {
		pthread_key_delete(_tls);
	}

	void* ThreadLocal::GetValue() const {
		return pthread_getspecific(_tls);
	}

	void ThreadLocal::SetValue(void* v) {
		pthread_setspecific(_tls, v);
	}
#endif

ThreadLocal::operator int() const {
	return (int)(long long)(void*)(GetValue());
}

void ThreadLocal::operator=(void* r) {
	SetValue(r);
}

void ThreadLocal::operator=(int r) {
	SetValue((void*)(r));
}

/** Wait **/
#ifdef TJ_OS_WIN
	void Wait::For(Thread& t, const Time& out) {
		For(t._thread, out);
	}

	void Wait::For(Event& e, const Time& out) {
		For(e._event, out);
	}

	void Wait::For(Semaphore& sm, const Time& out) {
		For(sm._sema, out);
	}


	Wait::Wait() {
	}

	Wait::~Wait() {
	}

	void Wait::Add(Thread& t) {
		_handles.push_back(t._thread);
	}

	void Wait::Add(Event& evt) {
		_handles.push_back(evt._event);
	}

	void Wait::Add(Semaphore& sm) {
		_handles.push_back(sm._sema);
	}

	bool Wait::ForAll(const Time& out) {
		unsigned int n = (unsigned int)_handles.size();
		HANDLE* handles = new HANDLE[n];
		std::vector<HANDLE>::iterator it = _handles.begin();
		for(unsigned int a=0;a<n;a++) {
			handles[a] = *it;
			++it;
		}

		bool r = For(handles, n, true, out) >= 0;	
		delete[] handles;
		return r;
	}

	int Wait::ForAny(const Time& out) {
		unsigned int n = (unsigned int)_handles.size();
		HANDLE* handles = new HANDLE[n];
		std::vector<HANDLE>::iterator it = _handles.begin();
		for(unsigned int a=0;a<n;a++) {
			HANDLE h = *it;
			handles[a] = h;
			++it;
		}

		int r = For(&(handles[0]), n, false, out);
		delete[] handles;
		return r;
	}

	void Wait::For(HANDLE handle, const Time& out) {
		DWORD tms = out.ToInt();
		if(tms<1) {
			tms = INFINITE;
		}
		
		switch(WaitForSingleObject(handle, tms)) {
			case WAIT_FAILED:
				Throw(L"Wait for single thread failed; invalid thread handle?", ExceptionTypeError);
				break;

			case WAIT_ABANDONED:
				Throw(L"Wait for single thread abandoned; this is bad.", ExceptionTypeError);
				break;

			default:
				break;
		}
	}

	int Wait::For(HANDLE* handles, unsigned int n, bool all, const Time& out) {
		DWORD timeOutMs = out.ToInt();
		if(timeOutMs<1) {
			timeOutMs = INFINITE;
		}

		int r = WaitForMultipleObjects(n, handles, all, timeOutMs);
		if(r==WAIT_ABANDONED) {
			Throw(L"Wait was abandoned; probably, a thread or event was destroyed while another thread was waiting on it through this waiting operation", ExceptionTypeError);
		}
		else if(r==WAIT_FAILED) {
			Throw(L"Wait failed; probably, an invalid handle or event was passed to this wait operation", ExceptionTypeError);
		}
		else if(r==WAIT_TIMEOUT) {
			return -1;
		}
		else {
			return r - WAIT_OBJECT_0;
		}
	}
#else
	#warning Not implemented (class Wait)
#endif

/** CriticalSection **/
#ifdef TJ_OS_WIN
	CriticalSection::CriticalSection() {
		InitializeCriticalSectionAndSpinCount(&_cs, 1024);
	}

	CriticalSection::~CriticalSection() {
		DeleteCriticalSection(&_cs);
	}

	void CriticalSection::Enter() {
		EnterCriticalSection(&_cs);
	}

	void CriticalSection::Leave() {
		LeaveCriticalSection(&_cs);
	}
#endif

#ifdef TJ_USE_PTHREADS
	CriticalSection::CriticalSection() {
		pthread_mutexattr_t attributes;
		pthread_mutexattr_init(&attributes);
		pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&_cs, &attributes);
		pthread_mutexattr_destroy(&attributes);
	}

	CriticalSection::~CriticalSection() {
		pthread_mutex_destroy(&_cs);
	}

	void CriticalSection::Enter() {
		pthread_mutex_lock(&_cs);
	}

	void CriticalSection::Leave() {
		pthread_mutex_unlock(&_cs);
	}
#endif

/** ThreadLock **/
ThreadLock::ThreadLock(CriticalSection *cs): _cs(cs) {
	_cs->Enter();
}

ThreadLock::ThreadLock(strong<Lockable> lockable): _cs(&(lockable->_lock)) {
	_cs->Enter();
}

ThreadLock::ThreadLock(Lockable* lockable): _cs(&(lockable->_lock)) {
	_cs->Enter();
}


ThreadLock::~ThreadLock() {
	_cs->Leave();
}

/** Lockable **/
Lockable::Lockable() {
}

Lockable::~Lockable() {
}

/** Runnable **/
Runnable::~Runnable() {
}
