#include "../include/tjshared.h"

DWORD WINAPI ThreadProc(LPVOID lpParam) {
	Thread* tr = (Thread*)lpParam;
	if(tr!=0) {
		tr->Run();
	}
	return 0;
}

/* Thread */
Thread::Thread() {
	_thread = CreateThread(NULL, 512, ThreadProc, (LPVOID)this, CREATE_SUSPENDED, (LPDWORD)&_id);
}

Thread::~Thread() {
	WaitForSingleObject(_thread, 0);
	CloseHandle(_thread);
}

void Thread::Start() {
	ResumeThread(_thread);
}

void Thread::Terminate() {
	TerminateThread(_thread, 0);
}

int Thread::GetID() {
	return _id;
}

void Thread::Run() {
}
