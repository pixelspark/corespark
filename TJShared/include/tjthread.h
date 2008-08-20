#ifndef _TJTHREAD_H
#define _TJTHREAD_H

namespace tj {
	namespace shared {
		class EXPORTED Runnable {
			public:
				virtual ~Runnable();
				virtual void Run() = 0;
		};

		class EXPORTED ThreadLocal {
			public:
				ThreadLocal();
				~ThreadLocal();
				int GetValue() const;
				void SetValue(int v);
				void operator=(int v);
				operator int() const;

			protected:
				#ifdef _WIN32
					DWORD _tls;
				#endif
		};

		class EXPORTED Semaphore {
			public:
				Semaphore();
				~Semaphore();
				void Release(int n = 1);
				bool Wait();

				#ifdef _WIN32
					HANDLE GetHandle();
				#endif

			private:
				#ifdef _WIN32
					HANDLE _sema;
				#endif
		};

		class EXPORTED Event {
			public:
				Event();
				~Event();
				void Signal();
				void Pulse();
				void Reset();
				void Wait(int ms=0);

				#ifdef _WIN32
					HANDLE GetHandle();
				#endif

			protected:
				#ifdef _WIN32
					HANDLE _event;
				#endif
		};

		class EXPORTED Thread: public virtual Object {
			#ifdef _WIN32
				friend DWORD WINAPI ThreadProc(LPVOID);
			#endif

			public:

				enum Priority {
					PriorityNormal = 0,
					PriorityIdle,
					PriorityAboveNormal,
					PriorityBelowNormal,
					PriorityHigh,
					PriorityLow,
					PriorityTimeCritical,
				};

			public:
				Thread();
				virtual ~Thread();
				virtual void Terminate();
				virtual void Start();
				void WaitForCompletion();
				int GetID() const;
				void SetName(const char* name);
				static long GetThreadCount();
				virtual void SetPriority(Priority p);

			protected:
				virtual void Run();
				
				#ifdef _WIN32
					HANDLE _thread;
				#endif
				
				int _id;
				bool _started;
				static volatile long _count;
		};

		class EXPORTED CriticalSection {
			friend class ThreadLock;

			public:
				CriticalSection();
				virtual ~CriticalSection();

			protected:
				void Enter();
				void Leave();

			private:
				#ifdef _WIN32
					CRITICAL_SECTION _cs;
				#endif
		};

		class EXPORTED ThreadLock {
			public:
				ThreadLock(CriticalSection* cs);
				virtual ~ThreadLock();

			protected:
				CriticalSection* _cs;
		};
	}
}

#endif