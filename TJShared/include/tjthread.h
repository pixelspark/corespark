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
			_param = param;
			SetEvent(_event);
		}

		void Reset() {
			ResetEvent(_event);
		}

		const T& Wait() {
			WaitForSingleObject(_event, 0);
			return _param;
		}

	protected:
		HANDLE _event;
		T _param;
};

class EXPORTED Thread {
	friend DWORD WINAPI ThreadProc(LPVOID);

	public:
		Thread();
		virtual ~Thread();
		virtual void Terminate();
		virtual void Start();
		int GetID();

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

		virtual void OnEvent(const T& param) {
		}

		Event<T> _event;
};

#endif