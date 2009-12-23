#include "../include/tjdispatch.h"
#include "../include/tjlog.h"
using namespace tj::shared;

/** Task **/
Task::~Task() {
}

Task::Task(): _flags(0) {
}

bool Task::IsRun() const {
	return (_flags & KTaskRun)!=0;
}

bool Task::IsEnqueued() const {
	return (_flags & KTaskEnqueued) != 0;
}

bool Task::DidFail() const {
	return (_flags & KTaskFailed) != 0;
}

/** Dispatcher **/
ref<Dispatcher> Dispatcher::_instance;

Dispatcher::Dispatcher(int maxThreads): _maxThreads(maxThreads), _busyThreads(0) {
	// TODO: if maxThreads=0, limit the maximum number of threads to the number of cores in the system * a load factor
	if(maxThreads==0) {
		_maxThreads = 2;
	}
}

Dispatcher::~Dispatcher() {
	Stop();
}

strong<Dispatcher> Dispatcher::DefaultInstance() {
	if(!_instance) {
		_instance = GC::Hold(new Dispatcher());
	}
	return _instance;
}

void Dispatcher::Stop() {
	// Send quit messages (null tasks)
	{
		ThreadLock lock(&_lock);

		// Dispatch the same quit task as many times as there are threads; this assumes
		// that threads do not take any new task after receiving a quit task
		std::set< ref<DispatchThread> >::iterator tit = _threads.begin();
		while(tit!=_threads.end()) {
			DispatchTask(null);
			++tit;
		}
	}

	_threads.clear(); // ~DispatcherThread will wait for completion
}

void Dispatcher::DispatchTask(ref<Task> t) {
	if(t && t->IsEnqueued()) {
		Throw(L"Task is already enqueued in (another?) dispatcher!", ExceptionTypeError);
	}

	ThreadLock lock(&_lock);
	if(t) {
		t->_flags |= Task::KTaskEnqueued;
	}
	_queue.push_back(t);
	_queuedTasks.Release();
}

void Dispatcher::Dispatch(strong<Task> t) {
	ThreadLock lock(&_lock);
	int numThreads = _threads.size();

	/* If there are no threads yet, or the number of busy threads is equal to the number of 
	available threads (i.e. all threads are busy) create a thread (if the total number of threads
	is still below the maximum number of threads */
	if((numThreads<1) || ((_busyThreads>=numThreads) && (int(_maxThreads)>numThreads))) {
		// Create a new response thread
		ref<DispatchThread> wrt = GC::Hold(new DispatchThread(this));
		_threads.insert(wrt);
		wrt->Start();
	}
	
	DispatchTask(t);
}

/** DispatchThread **/
DispatchThread::DispatchThread(ref<Dispatcher> d): _dispatcher(d) {
}

DispatchThread::~DispatchThread() {
	WaitForCompletion();
}

void DispatchThread::Run() {
	#ifdef TJ_OS_WIN
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
	#endif

	while(true) {
		ref<Dispatcher> dispatcher = _dispatcher;
		if(!dispatcher) {
			// The dispatcher has been destroyed and for some reason, we are still alive; quit too
			return;
		}

		Semaphore& queueSemaphore = dispatcher->_queuedTasks;
		dispatcher = null;

		// Wait for a task to appear in the queue; if there is a task, wake up and execute it
		if(queueSemaphore.Wait()) {
			ref<Task> task;

			{
				dispatcher = _dispatcher;
				if(!dispatcher) {
					return;
				}

				ThreadLock lock(&(dispatcher->_lock));	
				std::deque< ref<Task> >::iterator it = dispatcher->_queue.begin();
				if(it==dispatcher->_queue.end()) {
					continue;
				}

				task = *it;
				dispatcher->_queue.pop_front();
				if(task) {
					++(dispatcher->_busyThreads);
				}
				dispatcher = null;
			}

			try {
				if(task) {
					task->_flags &= (~Task::KTaskEnqueued);
					task->Run();
					task->_flags |= Task::KTaskRun;
				}
				else {
					return;
				}
			}
			catch(const Exception& e) {
				if(task) {
					task->_flags |= Task::KTaskFailed;
				}
				Log::Write(L"TJShared/DispatcherThread", L"Error occurred when processing client request: "+e.GetMsg());
			}
			catch(...) {
				if(task) {
					task->_flags |= Task::KTaskFailed;
				}
				Log::Write(L"TJShared/DispatcherThread", L"Unknown error occurred when processing client request");
			}

			{
				dispatcher = _dispatcher;
				if(!dispatcher) {
					return;
				}

				ThreadLock lock(&(dispatcher->_lock));	
				--(dispatcher->_busyThreads);
			}
		}
	}

	#ifdef TJ_OS_WIN
		CoUninitialize();
	#endif
}