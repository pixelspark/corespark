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

		class EXPORTED CriticalSection {
			friend class ThreadLock;

			public:
				CriticalSection();
				virtual ~CriticalSection();

			protected:
				void Enter();
				void Leave();

			private:
				#ifdef TJ_OS_WIN
					CRITICAL_SECTION _cs;
				#endif
			
				#ifdef TJ_OS_MAC
					pthread_mutex_t _cs;
				#endif
			
		};

		class EXPORTED Lockable {
			public:
				Lockable();
				virtual ~Lockable();
				CriticalSection _lock;
		};

		class EXPORTED ThreadLock {
			public:
				ThreadLock(CriticalSection* cs);
				ThreadLock(strong<Lockable> lockable);
				ThreadLock(Lockable* lockable);
				virtual ~ThreadLock();

			protected:
				CriticalSection* _cs;
		};

		class EXPORTED ThreadLocal {
			public:
				ThreadLocal();
				~ThreadLocal();
				void* GetValue() const;
				void SetValue(void* v);
				void operator=(void* v);
				void operator=(int v);
				operator int() const;

			protected:
				#ifdef TJ_OS_WIN
					DWORD _tls;
				#endif
			
				#ifdef TJ_OS_MAC
					pthread_key_t _tls;
				#endif
		};

		template<class T> class Singleton: public virtual Object {
			public:
				static inline strong<T> Instance() {
					static CriticalSection _initializationLock;
					static ref<T> _instance;

					if(!_instance) {
						ThreadLock lock(&_initializationLock);
						if(!_instance) {
							_instance = GC::Hold(new T());
						}
					}
					return _instance;
				}
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
				#ifdef TJ_OS_WIN
					HANDLE _event;
				#endif
			
				#ifdef TJ_OS_MAC
					pthread_cond_t _event;
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
				void SetName(const String& name);
				virtual void SetPriority(Priority p);

				static long GetThreadCount();
				static int GetCurrentThreadID();
				static String GetCurrentThreadName();
			
				static volatile ReferenceCount _count;


			protected:
				virtual void Run();
				static void Sleep(double ms);
				
				static CriticalSection _nameLock;
				static std::map<int, String> _names;

				#ifdef _WIN32
					HANDLE _thread;
				#else
					pthread_t _thread;
				#endif
				
				int _id;
				bool _started;
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
	}
}

#endif