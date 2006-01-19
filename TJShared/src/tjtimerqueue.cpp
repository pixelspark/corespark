#include "../include/tjshared.h"

void CALLBACK TimerQueueCallback(PVOID lpParameter,BOOLEAN TimerOrWaitFired) {
	TimerQueue::TimerData* timerData = (TimerQueue::TimerData*)lpParameter;
	if(timerData!=0) {
		DeleteTimerQueueTimer(timerData->_queue, timerData->_handle, 0);
		timerData->_timer->Fire(timerData->_timerQueue);
		delete timerData->_timer;
	}
	delete timerData;
}

Timer::Timer() {
}

Timer::~Timer() {
}

TimerQueue::TimerQueue() {
	_queue = CreateTimerQueue();
}

TimerQueue::~TimerQueue() {
	DeleteTimerQueue(_queue);
}

void TimerQueue::AddTimer(Timer* t, unsigned int ms) {
	//ThreadLock lck(_lock);
	TimerQueue::TimerData* timerData = new TimerQueue::TimerData();
	timerData->_timer = t;
	timerData->_queue = _queue;
	timerData->_handle = 0;
	timerData->_timerQueue = this;
	CreateTimerQueueTimer(&timerData->_handle, _queue, TimerQueueCallback, (void*)timerData, ms, 0, WT_EXECUTEONLYONCE);
}