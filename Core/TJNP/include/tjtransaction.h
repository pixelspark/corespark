#ifndef _TJ_NP_TRANSACTION_H
#define _TJ_NP_TRANSACTION_H

#include "tjnpinternal.h"
#include "tjprotocol.h"

#pragma warning(push)
#pragma warning(disable: 4275)

namespace tj {
	namespace np {
		class NP_EXPORTED Transaction: public virtual tj::shared::Object {
			public:
				Transaction(tj::shared::Time out = 5000);
				virtual ~Transaction();
				virtual bool IsExpired() const;
				virtual void OnReceive(int instance, in_addr from, const PacketHeader& header, tj::shared::ref<tj::shared::DataReader> packet) = 0;
				virtual void OnExpire();

			protected:
				unsigned int _expires;
		};

		/** A node is the 'implementation part' of the protocol. An application that wants to connect using
		TNP should implement a 'Node'. A Node is actually an extended transaction handler (Transaction class)
		with some extra methods for node identification etc. **/
		class NP_EXPORTED Node: public Transaction {
			public:
				virtual ~Node();
				virtual InstanceID GetInstanceID() const = 0;
		};
	}
}

#pragma warning(pop)
#endif