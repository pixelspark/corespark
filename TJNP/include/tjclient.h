#ifndef _TJ_NP_CLIENT_H
#define _TJ_NP_CLIENT_H

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
				const tj::shared::Networking::MACAddress& GetMACAddress() const;

				static std::wstring GetHostName(const std::wstring& ip);
				static std::wstring GetHostName(const in_addr& addr);

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
				tj::shared::Networking::MACAddress _mac;
		};
	}
}

#pragma warning(pop)
#endif