#ifndef _TJTHREAD_H
#define _TJTHREAD_H

template<typename T=int> class Event {
	public:
		Event() {
			_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		~Event() {
			CloseHandle(_event);
		}

		void Signal(T param) {
			WaitForSingleObject(_event,0); // wait for previous event to complete
			SetEvent(_event);
		}

		void Reset() {
			ResetEvent(_event);
		}

		const T& Wait() {
			WaitForSingleObject(_event, 0);
		}

	protected:
		HANDLE _event;
};

class EXPORTED Thread {
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
};

template<typename T> class EventThread: public Thread {
	public:
		EventThread() {
		}

		virtual ~EventThread() {
		}

		void Signal(T param) {
			_event.Signal(param);
		}

	protected:
		virtual void Run() {
			const T& param = _event.Wait();
			OnEvent(param);
			_event.Reset();
		}

		virtual void OnEvent(const T& param)=0;

		Event<T> _event;
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

#endif