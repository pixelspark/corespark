#include "../include/tjendpoint.h"
#include "../include/tjlanguage.h"
#include "../include/tjthread.h"
#include "../include/tjlog.h"
#include "../include/tjdispatch.h"

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

/* RecycleableResource */
intern::RecycleableResource::RecycleableResource(RecycleBin& rc): _bin(rc) {
}

intern::RecycleableResource::~RecycleableResource() {
}

/* Object */
Object::Object(): _resource(0) {
}

Object::~Object() {
}

void Object::OnCreated() {
}

/* Recycleable */
Recycleable::~Recycleable() {
}

void Recycleable::OnRecycle() {
}

void Recycleable::OnReuse() {
}

/* RecycleBin */
unsigned int RecycleBin::_totalObjectCount = 0;

unsigned int RecycleBin::GetTrashObjectCount() {
	return _totalObjectCount;
}

RecycleBin::RecycleBin(): _lock(new CriticalSection()), _added(0), _removed(0) {
	for(unsigned int a=0; a<KRecycleBinMaximumSize; a++) {
		_bin[a] = 0;
	}
}

RecycleBin::~RecycleBin() {
	ThreadLock lock(_lock);
	
	for(unsigned int a=_removed; a<_added; a++) {
		delete _bin[a % KRecycleBinMaximumSize]->_resource;
		delete _bin[a % KRecycleBinMaximumSize];
	}
	
	delete _lock;
}

void RecycleBin::Reuse(Recycleable* rc) {
	if(rc==0) {
		throw NullPointerException();
	}
	
	if(rc->_resource->IsReferenced() || rc->_resource->IsWeaklyReferenced()) {
		throw BadReferenceException();
	}
	
	ThreadLock lock(_lock);
	if(WantsToRecycle()) {
		try {
			rc->OnRecycle();
		}
		catch(...) {
			delete rc->_resource;
			delete rc;
			return;
		}
		
		_bin[_added % KRecycleBinMaximumSize] = rc;
		++_added;
		++_totalObjectCount;
	}
	else {
		delete rc->_resource;
		delete rc;
	}
}

Recycleable* RecycleBin::Get() {
	{
		ThreadLock lock(_lock);
		if(_removed<_added) {
			Recycleable* rc = _bin[_removed % KRecycleBinMaximumSize];
			_bin[_removed % KRecycleBinMaximumSize] = 0;
			++_removed;
			--_totalObjectCount;
			return rc;
		}
	}
	return 0;
}

bool RecycleBin::WantsToRecycle() const {
	return (_added-_removed) < KRecycleBinMaximumSize;
}

/* Serializable */
Serializable::~Serializable() {
}

/** OutOfMemoryException **/
OutOfMemoryException::OutOfMemoryException(): Exception(L"Out of memory!", ExceptionTypeError) {
}

OutOfMemoryException::~OutOfMemoryException() {
}

/** BadCastException **/
BadCastException::BadCastException(): Exception(L"A bad cast was attempted", ExceptionTypeError) {
}

BadCastException::~BadCastException() {
}

/** BadReferenceException **/
BadReferenceException::BadReferenceException(): Exception(L" A reference error has occurred", ExceptionTypeError) {
}

BadReferenceException::~BadReferenceException() {
}

/** NullPointerException **/
NullPointerException::NullPointerException(): Exception(L"A null pointer was dereferenced", ExceptionTypeError) {
}

NullPointerException::~NullPointerException() {
}

/** StrongReferenceException **/
StrongReferenceException::StrongReferenceException(): Exception(L"A null reference tried to become a strong reference", ExceptionTypeError) {
}

StrongReferenceException::~StrongReferenceException() {
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
		
		// Put the child process in a new process group to make it independent
		setsid();
	
		/* Fork again, because the just created process is a session leader, and
		the first tty it opens becomes the controlling terminal, which is not 
		desirable. */
		i = fork();
		if(i<0) {
			return false; // fork error
		}
		else if(i>0) {
			return false; // This is the parent process, can exit
		}
	
		// Set log-to-syslog instead of log-to-console
		Log::SetLogToSyslog(true);
		Log::SetLogToConsole(false);
	
		// chdir to /tmp to make sure that we're not locking up some directory we might be in
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
	#ifdef TJ_OS_WIN
		signal(SIGINT, SignalHandler);
	#endif

	#ifdef TJ_OS_POSIX
		struct sigaction sa;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;	
		sa.sa_handler = SignalHandler;

		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGCHLD, &sa, NULL);
		sigaction(SIGUSR1, &sa, NULL);
		signal(SIGPIPE, SIG_IGN);
	#endif
	
	#ifdef TJ_OS_MAC
		sigaction(SIGINFO, &sa, NULL);
	#endif
	
	// Wait for stop (interruption with SIGINT will signal _globalStop)
	while(true) {
		_signalEvent.Wait();
		if(_lastSignal==SIGINT) {
			break;
		}
		
		#ifdef TJ_OS_POSIX
			#ifdef TJ_OS_MAC
				else if(_lastSignal==SIGINFO) {
			#else
				else if(_lastSignal==SIGUSR1) {
			#endif
			std::wostringstream info;
					info << L"GC: " << tj::shared::intern::Resource::GetResourceCount() << L" Threads: " << Thread::GetThreadCount() << L" Trash: " << RecycleBin::GetTrashObjectCount() << L" Locks: " << CriticalSection::GetCriticalSectionCount();
			Log::Write(L"TJShared/Daemon", info.str());
		}
		#endif
		
		#ifdef TJ_OS_POSIX
		else if(_lastSignal==SIGHUP) {
			// Reload config?
			Log::Write(L"TJShared/Daemon", L"Received hang-up signal");
		}
		else if(_lastSignal==SIGCHLD) {
			Log::Write(L"TJShared/Daemon", L"A child process terminated");
		}
		#endif
		
		else {
			Log::Write(L"TJShared/Daemon", L"A signal was received ("+StringifyHex(_lastSignal)+L"), ignoring");
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
