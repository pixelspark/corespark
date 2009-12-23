#ifndef _TJ_DISPATCH_H
#define _TJ_DISPATCH_H

#include "tjsharedinternal.h"
#include "tjutil.h"
#include "tjthread.h"
#include <deque>

namespace tj {
	namespace shared {
		class EXPORTED Task: public virtual Object, public Runnable {
			friend class DispatchThread;
			friend class Dispatcher;

			public:
				virtual ~Task();
				bool IsRun() const;
				bool IsEnqueued() const;
				bool DidFail() const;
				virtual void Run() = 0;

			protected:
				Task();
				volatile int _flags;
				const static int KTaskEnqueued = 0x1;
				const static int KTaskRun = 0x2;
				const static int KTaskFailed = 0x4;
		};

		class DispatchThread;

		class EXPORTED Dispatcher: public virtual Object {
			friend class DispatchThread;

			public:
				Dispatcher(int maxThreads = 0);
				virtual ~Dispatcher();
				static strong<Dispatcher> DefaultInstance();
				virtual void Dispatch(strong<Task> t);
				virtual void Stop();

			private:
				virtual void DispatchTask(ref<Task> t);

				CriticalSection _lock;
				std::deque< ref<Task> > _queue;
				Semaphore _queuedTasks;
				std::set< ref<DispatchThread> > _threads;
				int _maxThreads;
				volatile int _busyThreads;

				static ref<Dispatcher> _instance;
		};

		class EXPORTED DispatchThread: public Thread {
			public:
				DispatchThread(ref<Dispatcher> d);
				virtual ~DispatchThread();
				virtual void Run();

			private:
				weak<Dispatcher> _dispatcher;
		};
	}
}

#endif