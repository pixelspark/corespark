#ifndef _TJ_NP_SOCKET_H
#define _TJ_NP_SOCKET_H

struct sockaddr_in;
#pragma pack(push,1)

namespace tj {
	namespace np {
		class NetworkInitializer {
			public:
				NetworkInitializer();
				~NetworkInitializer();
				void Initialize();

			protected:
				void* _data;
		};

		class NP_EXPORTED Socket {
			public:
				Socket(int port, const char* address, tj::shared::ref<Node> main);
				virtual ~Socket();
			
				void SendAnnounce(Role r, const std::wstring& address, Features feats, tj::shared::strong<Transaction> ti);
				void SendAnnounceReply(Role r, const std::wstring& address, Features feats, TransactionIdentifier ti);
				void SendLeave();
				void SendDemoted();
				void SendPromoted();
				void SendResourcePush(const GroupID& gid, const tj::shared::ResourceIdentifier& rid);
				void SendResourceFind(const tj::shared::ResourceIdentifier& ident, tj::shared::ref<Transaction> tr = 0);
				void SendResourceAdvertise(const tj::shared::ResourceIdentifier& rid, const std::wstring& url, unsigned short port, TransactionIdentifier tid = 0);
				void SendError(Features involved, tj::shared::ExceptionType type, const std::wstring& message);
				void SendListDevices(InstanceID to, tj::shared::ref<Transaction> ti);
				void SendListDevicesReply(const DeviceIdentifier& di, const std::wstring& friendly, TransactionIdentifier ti, in_addr to, unsigned int count);
				void SendListPatches(InstanceID to, tj::shared::ref<Transaction> ti);
				void SendListPatchesReply(const PatchIdentifier& pi, const DeviceIdentifier& di, TransactionIdentifier ti, in_addr to, unsigned int count);
				void SendResetAll();
				void SendSetPatch(tj::shared::ref<BasicClient> c, const PatchIdentifier& pi, const DeviceIdentifier& di);
				void SendSetClientAddress(tj::shared::ref<BasicClient> c, std::wstring newAddress);
				void SendInput(const PatchIdentifier& patch, const InputID& path, float value);
				void SendOutletChange(Channel ch, GroupID gid, const std::wstring& outletName, const tj::shared::Any& value);
				void SendResetChannel(GroupID gid, Channel ch);

				void Send(tj::shared::strong<Message> s, bool reliable = false);
				void Send(tj::shared::strong<Packet> p, bool reliable = false);
				int GetPort() const;
				std::wstring GetAddress() const;
				int GetBytesSent() const;
				int GetBytesReceived() const;
				unsigned int GetActiveTransactionCount() const;
				unsigned int GetWishListSize() const;
				void CleanTransactions();
				void SendRedeliveryRequests();

				// Called by network implementation layer, do not call by yourself
				void Receive();
			
			private:
				ReliablePacketID RegisterReliablePacket(tj::shared::strong<Packet> p);
				void Send(tj::shared::strong<Packet> p, const sockaddr_in* address, bool reliable);
				
				static NetworkInitializer _initializer;

				HWND _window;
				SOCKET _server;
				SOCKET _client;
				char* _recieveBuffer;
				char* _bcastAddress;
				int _port;
				int _bytesSent;
				int _bytesReceived;
				unsigned int _maxReliablePacketCount;
				ReliablePacketID _lastPacketID;
				TransactionIdentifier _transactionCounter;
				tj::shared::weak<Node> _network;
				std::map< TransactionIdentifier, tj::shared::ref<Transaction> > _transactions;
				std::map< ReliablePacketID, tj::shared::ref<Packet> > _reliableSentPackets;
				std::deque< std::pair<InstanceID, ReliablePacketID> > _reliableWishList;
				std::map< InstanceID, ReliablePacketID > _reliableLastReceived;
				mutable tj::shared::CriticalSection _lock;
				
				const static unsigned int KDefaultMaxReliablePacketCount = 50;
		};
	}
}

#pragma pack(pop)
#endif