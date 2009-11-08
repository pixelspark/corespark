#ifndef _TJTHREAD_H
#define _TJTHREAD_H

#include "internal/tjpch.h"
#include "tjtime.h"
#include <map>
#include <vector>

namespace tj {
	namespace shared {
		class EXPORTED Atomic {
			public:
				static inline long Exchange(volatile long* target, long value) {
					#ifdef TJ_OS_WIN
						return InterlockedExchange(target, value);
					#endif
					
					#ifdef TJ_OS_MAC
						// Capture the old value, and then atomically compare-and-swap
						// If the value has changed in the mean time, try again
						while(true) {
							long oldValue = *target;
							if(OSAtomicCompareAndSwapLong(oldValue, value, target)) {
								return oldValue;
							}
						}
					#endif
					
					#ifdef TJ_OS_LINUX
						long oldValue = *target;
						*target = value;
						return oldValue;
						// There is a #warning in tjthread.cpp to alert that this is not really atomic...
						// Should use __sync_add_and_fetch or something like that on GCC4.2, but this is
						// not currently supported on ARM
					#endif
				}
		};
		
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
			
				#ifdef TJ_OS_POSIX
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
			
				#ifdef TJ_OS_POSIX
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
				bool Wait(const Time& out = Time(-1));

				#ifdef TJ_OS_WIN
					HANDLE GetHandle();
				#endif

			private:
				#ifdef TJ_OS_WIN
					HANDLE _sema;
				#endif
			
				#ifdef TJ_OS_POSIX
					void* _sema;
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
				bool Wait(int ms=0);

				#ifdef TJ_OS_WIN
					HANDLE GetHandle();
				#endif

			protected:
				#ifdef TJ_OS_WIN
					HANDLE _event;
				#endif
			
				#ifdef TJ_OS_POSIX
					pthread_cond_t _event;
					pthread_mutex_t _lock;
					volatile int _signalCount;
				#endif
		};

		class EXPORTED Thread: public virtual Object {
			friend class Wait;

			#ifdef TJ_OS_WIN
				friend DWORD WINAPI ThreadProc(LPVOID);
			#else
				friend void* ThreadProc(void* arg);
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

				#ifdef TJ_OS_WIN
					HANDLE _thread;
				#endif
				
				#ifdef TJ_OS_POSIX
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

				bool ForAll(const Time& out = Time(-1));
				int ForAny(const Time& out = Time(-1));
				
				template<typename T> inline Wait& operator[](T& t) {
					Add(t);
					return *this;
				}

			private:
				#ifdef TJ_OS_WIN
					static void For(HANDLE h, const Time& out);
					static int For(HANDLE* handles, unsigned int n, bool all, const Time& out);
					std::vector<HANDLE> _handles;
				#endif
		};
		
		class EXPORTED Daemon: public virtual Object {
			public:
				static strong<Daemon> Instance();
				virtual ~Daemon();
				virtual void Run();
				virtual bool Fork(const String& daemonName, bool singleInstance);
				
			protected:
				Daemon();
				static ref<Daemon> _instance;
				static Event _signalEvent;
				static int _lastSignal;
				static void SignalHandler(int s);
		};
	}
}

#endif