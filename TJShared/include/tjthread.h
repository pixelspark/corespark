#ifndef _TJTHREAD_H
#define _TJTHREAD_H

#include "tjtime.h"

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
			friend class Wait;

			public:
				Semaphore();
				~Semaphore();
				void Release(int n = 1);
				bool Wait();

			private:
				#ifdef _WIN32
					HANDLE _sema;
				#endif
		};

		class EXPORTED Event {
			friend class Wait;
			friend class Core;
			friend class SplashThread;

			public:
				Event();
				~Event();
				void Signal();
				void Pulse();
				void Reset();
				void Wait(int ms=0);

			protected:
				#ifdef _WIN32
					HANDLE _event;
				#endif
		};

		class EXPORTED PeriodicTimer {
			friend class Wait;

			public:
				PeriodicTimer();
				~PeriodicTimer();
				void Start(const Time& period);

			protected:
				#ifdef WIN32
					HANDLE _timer;
				#endif
		};

		class EXPORTED Thread: public virtual Object {
			friend class Wait;

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

		class EXPORTED Wait {
			public:
				static void For(Thread& t, const Time& out = Time(-1));
				static void For(Event& e, const Time& out = Time(-1));
				static void For(Semaphore& s, const Time& out = Time(-1));

				Wait();
				~Wait();
				void Add(Thread& t);
				void Add(Event& evt);
				void Add(Semaphore& smp);
				void Add(PeriodicTimer& pt);

				bool ForAll(const Time& out = Time(-1));
				int ForAny(const Time& out = Time(-1));
				
				template<typename T> inline Wait& operator[](T& t) {
					Add(t);
					return *this;
				}

			private:
				#ifdef WIN32
					static void For(HANDLE h, const Time& out);
					static int For(HANDLE* handles, unsigned int n, bool all, const Time& out);
					std::vector<HANDLE> _handles;
				#endif
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