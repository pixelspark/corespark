/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #include "../include/tjtransaction.h"

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