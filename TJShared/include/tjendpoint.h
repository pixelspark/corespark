#ifndef _TJENDPOINT_H
#define _TJENDPOINT_H

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
				virtual std::wstring GetName() const;
		};
	}
}

#endif