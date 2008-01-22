#ifndef _TJTHREAD_H
#define _TJTHREAD_H

namespace tj {
	namespace shared {
		class EXPORTED Runnable {
			public:
				virtual ~Runnable();
				virtual void Run() = 0;
		};

		class EXPORTED Event {
			public:
				Event() {
					_event = CreateEvent(NULL, TRUE, FALSE, NULL);
				}

				~Event() {
					CloseHandle(_event);
				}

				void Signal() {
					SetEvent(_event);
				}

				void Reset() {
					ResetEvent(_event);
				}

				HANDLE GetHandle() {
					return _event;
				}

				void Wait(int ms=0) {
					WaitForSingleObject(_event, ms);
				}

			protected:
				HANDLE _event;
		};

		class EXPORTED Thread: public virtual Object {
			friend DWORD WINAPI ThreadProc(LPVOID);

			public:
				Thread();
				virtual ~Thread();
				virtual void Terminate();
				virtual void Start();
				void WaitForCompletion();
				int GetID() const;
				void SetName(const char* name);

			protected:
				virtual void Run();
				HANDLE _thread;
				int _id;
				bool _started;
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
				CRITICAL_SECTION _cs;
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