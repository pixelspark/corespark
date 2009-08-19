#include "../include/tjshared.h"
using namespace tj::shared;

ThreadLocal Zone::_globalBarrier;

Zone::Zone() {
}

Zone::~Zone() {
}

/* A difference between Deny() and GlobalDeny() is that the global version
 doesn't check if the thread is already in a zone. Thus, GlobalDeny should
 always be called at the beginning of a thread that executes potentially
 dangerous code (e.g. scripts) to 'sandbox'  them. */
void Zone::GlobalDeny() {
	int old = _globalBarrier;
	_globalBarrier = _globalBarrier + 1;

	if(old>_globalBarrier) {
		Throw(L"Zone overflow error!", ExceptionTypeSevere);
	}
}
 
void Zone::GlobalAllow() {
	if(_globalBarrier>0) {
		int old = _globalBarrier;
		_globalBarrier = _globalBarrier - 1;

		if(old<_globalBarrier) {
			Throw(L"Zone overflow error!", ExceptionTypeSevere);
		}
	}
	else {
		Throw(L"Cannot unsandbox thread, since it was not sandboxed!", ExceptionTypeSevere);
	}
}

void Zone::Deny() {
	int old = _barrier;
	_barrier = old+1;

	/* When Deny gets called more than INT_MAX times, the counter
	may overflow and a thread could be denied access. To prevent
	exploitation of this, check here. */
	if(old > _barrier) {
		Throw(L"Zone overflow error!", ExceptionTypeSevere);
	}
}

void Zone::Allow() {
	if(_barrier<=0) {
		// trying to allow access more than it was denied
		Throw(L"Zone access is already allowed!", ExceptionTypeError);
	}

	int old = _barrier;
	_barrier = old-1;
	/* When Allow gets called more than INT_MAX times, the counter
	may overflow and a thread could be denied access. To prevent
	exploitation of this, check here. */
	if(old < _entry) {
		Throw(L"Zone overflow error!", ExceptionTypeSevere);
	}
}

bool Zone::CanEnter() {
	return _barrier==0 && _globalBarrier==0;
}

void Zone::Leave() {
	if(_entry>0) {
		int old = _entry;
		_entry = _entry - 1;

		/* When Leave gets called more than INT_MAX times, the counter
		may overflow and a thread could be allowed access. To prevent
		exploitation of this, check here. */
		if(old < _entry) {
			Throw(L"Zone overflow error!", ExceptionTypeSevere);
		}
	}
	else {
		Throw(L"Thread is not in this zone, cannot leave", ExceptionTypeError);
	}
}

bool Zone::IsInside() const {
	return (_entry > 0);
}

void Zone::Enter() {
	if(CanEnter()) {
		int old = _entry;
		_entry = _entry + 1;

		/* When Allow gets called more than INT_MAX times, the counter
		may overflow and a thread could be denied access. To prevent
		exploitation of this, check here. */
		if(old > _entry) {
			Throw(L"Zone overflow error!", ExceptionTypeSevere);
		}
	}
	else {
		Throw(L"Access to zone denied!", ExceptionTypeError);
	}
}

Zone GZones[Zones::_LastZone];

Zone& Zones::Get(const Zones::PredefinedZone& pz) {
	return GZones[pz];
}

bool Zones::IsDebug() {
	Zone& z = Get(DebugZone);
	return z.IsInside();	
}