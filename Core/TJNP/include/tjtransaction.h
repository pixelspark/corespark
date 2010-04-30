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