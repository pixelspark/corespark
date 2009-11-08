#include "../include/internal/tjpch.h"
#include "../include/tjendpoint.h"
#include "../include/tjlanguage.h"
#include "../include/tjthread.h"
#include "../include/tjlog.h"

#ifdef TJ_OS_MAC
	#include <CoreFoundation/CoreFoundation.h>
	#include <sys/stat.h>
#endif

#include <signal.h>

#ifdef TJ_OS_POSIX
	#include <sys/types.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <fcntl.h>
#endif

using namespace tj::shared;

volatile ReferenceCount intern::Resource::_resourceCount = 0L;

/* Exception */
Exception::Exception(const String& message, ExceptionType type, const char* file, int line) {
	_message = message;
	_type = type;
	_file = file;
	_line = line;
}

Exception::~Exception() {
}

String Exception::ToString() const {
	std::wostringstream os;
	os << _message;
	os << " (" << _file << ":" << _line << ")";
	
	return os.str();
}

const String& Exception::GetMsg() const {
	return _message;
}

ExceptionType Exception::GetType() const {
	return _type;
}

int Exception::GetLine() const {
	return _line;
}

const char* Exception::GetFile() const {
	return _file;
}

/* GC */
void GC::Log(const char* tp, bool allocate) {
	#ifdef TJ_OS_WIN
		if(allocate) {
				OutputDebugString(L"A ");
		}
		else {
			OutputDebugString(L"D ");
		}

		OutputDebugStringA(tp);
		OutputDebugString(L"\r\n");
	#endif
	
	#ifdef TJ_OS_MAC
		std::cout << (allocate?"A":"D") << " " << tp << std::endl;
	#endif
}

/* Endpoint */
Endpoint::EndpointType Endpoint::GetType() const {
	return EndpointTypeThreaded;
}

Endpoint::~Endpoint() {
}

String Endpoint::GetName() const {
	return TL(endpoint_unknown);
}

/* Resource */
intern::Resource::Resource(): _referenceCount(0), _weakReferenceCount(0) {
	#ifdef TJ_OS_WIN
		InterlockedIncrement(&_resourceCount);
	#endif
	
	#ifdef TJ_OS_MAC
		OSAtomicAdd32(1, &_resourceCount);
	#endif
	
	#ifdef TJ_OS_LINUX
		_resourceCount++;
	#endif
}

intern::Resource::~Resource() {
	#ifdef TJ_OS_WIN
		InterlockedDecrement(&_resourceCount);
	#endif
	
	#ifdef TJ_OS_MAC
		OSAtomicAdd32(-1, &_resourceCount);
	#endif
	
	#ifdef TJ_OS_LINUX
		_resourceCount--;
	#endif
}

long intern::Resource::GetResourceCount() {
	return _resourceCount;
}

/* Object */
void Object::OnCreated() {
}

/* Serializable */
Serializable::~Serializable() {
}

/** OutOfMemoryException **/
OutOfMemoryException::OutOfMemoryException(): Exception(L"Out of memory!", ExceptionTypeError) {
}

/** Daemon **/
ref<Daemon> Daemon::_instance;
Event Daemon::_signalEvent;
int Daemon::_lastSignal = 0;

Daemon::Daemon() {
	Log::SetLogToConsole(true);
}

Daemon::~Daemon() {
}

strong<Daemon> Daemon::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new Daemon());
	}
	return _instance;
}

bool Daemon::Fork(const String& daemonName, bool singleInstance) {
	// Run as daemon
	#ifdef TJ_OS_POSIX
		Log::Write(L"TJShared/Daemon", L"Will run as daemon");
		// Fork off a child process that continues to run, while the parent exits
		int i = fork();
		if(i<0) {
			return false; // fork error
		}
		else if(i>0) {
			return false; // This is the parent process, can exit
		}
		
		// Make child process independent
		setsid();
		chdir("/tmp");
		
		// Restrict file creation (mode will be 750)
		umask(027);
		
		// Let's see if we're the only ones running
		std::ostringstream pidFile;
		pidFile << "/var/run/" << Mbs(daemonName) << ".pid";
		std::string pidFilePath = pidFile.str();
		
		int lfp = open(pidFilePath.c_str(), O_RDWR|O_CREAT, 0640);
		if (lfp<0) {
			Log::Write(L"TJShared/Daemon", L"Cannot open lock file");
			return false;
		}
		
		if(lockf(lfp,F_TLOCK,0)<0) {
			Log::Write(L"TJShared/Daemon", L"Cannot lock file; is another instance already running?");
			return false;
		}
		
		// save PID to lock file
		std::ostringstream wos;
		wos << getpid() << '\n';
		std::string pidString = wos.str();
		write(lfp, pidString.c_str(),pidString.length());
		
		// Child process is good to go
		return true;
	#else
		Log::Write(L"TJShared/Daemon", L"This platform does not support running a daemon process this way; running in the parent process");
		return true;
	#endif
}

void Daemon::Run() {
	// Set up signal handlers
	signal(SIGINT, SignalHandler);
	
	#ifdef TJ_OS_POSIX
		signal(SIGHUP, SignalHandler);
		signal(SIGCHLD, SignalHandler);
	#endif
	
	#ifdef TJ_OS_MAC
		signal(SIGINFO, SignalHandler);
	#endif
	
	// Wait for stop (interruption with SIGINT will signal _globalStop)
	while(true) {
		_signalEvent.Wait();
		if(_lastSignal==SIGINT) {
			break;
		}
		
		#ifdef TJ_OS_MAC
		else if(_lastSignal==SIGINFO) {
			std::wostringstream info;
			info << L"GC: " << tj::shared::intern::Resource::GetResourceCount() << L"Threads: " << Thread::GetThreadCount();
			Log::Write(L"TJShared/Daemon", info.str());
		}
		#endif
		
		#ifdef TJ_OS_POSIX
		else if(_lastSignal==SIGHUP) {
			// Reload config?
			Log::Write(L"TJShared/Daemon", L"Received hang-up signal; will reload configuration");
		}
		else if(_lastSignal==SIGCHLD) {
			Log::Write(L"TJShared/Daemon", L"A child process terminated");
		}
		#endif
		
		else {
			break;
		}
	}
	
	// Set up signal handlers
	signal(SIGINT, 0);
	
	#ifdef TJ_OS_POSIX
		signal(SIGHUP, 0);
		signal(SIGCHLD, 0);
	#endif
		
	#ifdef TJ_OS_MAC
		signal(SIGINFO, 0);
	#endif
}

void Daemon::SignalHandler(int s) {
	_lastSignal = s;
	_signalEvent.Signal();
}