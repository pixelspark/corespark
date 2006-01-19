#ifndef _TJTIMERQUEUE_H
#define _TJTIMERQUEUE_H

class TimerQueue;

class EXPORTED Timer {
	public:
		virtual void Fire(TimerQueue* tq) = 0;
		virtual ~Timer();

	protected:
		Timer();
};

class EXPORTED TimerQueue {
	friend void CALLBACK TimerQueueCallback(PVOID,BOOLEAN);

	public:
		TimerQueue();
		virtual ~TimerQueue();
		void AddTimer(Timer* t, unsigned int ms);
	protected:
		HANDLE _queue;

		struct TimerData {
			Timer* _timer;
			HANDLE _handle;
			HANDLE _queue;
			TimerQueue* _timerQueue;
		};
};

#endif