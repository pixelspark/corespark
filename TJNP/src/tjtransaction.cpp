#include "../include/tjnp.h"

#ifdef _WIN32
	#include <windows.h> 
#endif

using namespace tj::np;
using namespace tj::shared;

/** Transaction **/
Transaction::Transaction(tj::shared::Time out) {
	#ifdef TJ_OS_WIN
		_expires = GetTickCount() + out.ToInt();
	#else
		#ifdef TJ_OS_POSIX
			_expires = time(NULL) + out.ToInt();
		#else
			#error Not implemented
		#endif
	#endif
}

Transaction::~Transaction() {
}

bool Transaction::IsExpired() const {
	#ifdef _WIN32
		return GetTickCount() > _expires;
	#else
		#ifdef TJ_OS_POSIX
			return time(NULL) > _expires;
		#else
			#error Not implemented
		#endif
	#endif
}

void Transaction::OnExpire() {
}

/* Node */
Node::~Node() {
}