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
 
 #ifndef _TJENDPOINT_H
#define _TJENDPOINT_H

#include "tjsharedinternal.h"
#include "tjany.h"

namespace tj {
	namespace shared {
		// Event recieving interface
		class EXPORTED Endpoint {
			public:
				// if EndpointTypeThreaded, the endpoint can be 'set' from any thread
				//		(faster, less safe, synchronous/blocking input, low latency)
				// if EndpointTypeMainThread, a runnable is posted to the core and the
				//		endpoint is set from the main thread 
				//		(slower, safer, no locking required, latency, blocking UI)
				enum EndpointType {
					EndpointTypeThreaded=1,
					EndpointTypeMainThread,
				};

				virtual ~Endpoint();
				virtual EndpointType GetType() const;
				virtual void Set(const Any& v) = 0;
				virtual String GetName() const;
		};
	}
}

#endif