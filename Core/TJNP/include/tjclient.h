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
 
 #ifndef _TJ_NP_CLIENT_H
#define _TJ_NP_CLIENT_H

#include "tjnpinternal.h"
#include "tjprotocol.h"
#include "tjnetwork.h"

#pragma warning(push)
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)

namespace tj {
	namespace np {
		class NP_EXPORTED BasicClient: public virtual tj::shared::Object, public virtual tj::shared::Serializable {
			public:
				BasicClient(Role r, const tj::shared::Timestamp& lastSeen, const std::wstring& ip, const std::wstring& addressing, const InstanceID& instance);
				virtual ~BasicClient();

				Role GetRole() const;
				tj::shared::Timestamp GetLastSeen() const;
				std::wstring GetIP() const;
				void SetIP(const std::wstring& ip);
				std::wstring GetAddressing() const;
				InstanceID GetInstanceID() const;
				void SetInstanceID(const InstanceID& id);
				std::wstring GetHostName();
				void SetLastSeen(const tj::shared::Timestamp& ls);
				void SetLastAnnounce(const tj::shared::Timestamp& la);
				void SetRole(Role r);
				void SetAddressing(const std::wstring& a);
				bool IsOnline(tj::shared::Time limit) const; // Set limit to be larger than the announce interval length
				long double GetLatency() const;
				tj::shared::Timestamp GetLastAnnounce() const;
				Features GetFeatures() const;
				void SetFeatures(Features f);
				const Networking::MACAddress& GetMACAddress() const;
				virtual void Save(TiXmlElement* me);
				virtual void Load(TiXmlElement* you);

			private:
				Role _role;
				tj::shared::Timestamp _lastSeen;
				tj::shared::Timestamp _lastAnnounce;
				std::wstring _ip;
				std::wstring _addressing;
				std::wstring _hostName;
				int _instance;
				Features _features;
				Networking::MACAddress _mac;
		};
	}
}

#pragma warning(pop)
#endif