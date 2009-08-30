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
		#error Not implemented
	#endif
}

Transaction::~Transaction() {
}

bool Transaction::IsExpired() const {
	#ifdef _WIN32
		return GetTickCount() > _expires;
	#else
		#error Not implemented
	#endif
}

void Transaction::OnExpire() {
}

/* Node */
Node::~Node() {
}