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
				tj::shared::CriticalSection _lock;
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
				void SendResourcePush(Channel c, const tj::shared::ResourceIdentifier& rid);
				void SendResourceFind(const tj::shared::ResourceIdentifier& ident, tj::shared::ref<Transaction> tr = 0);
				void SendResourceAdvertise(const tj::shared::ResourceIdentifier& rid, const std::wstring& url, unsigned short port, TransactionIdentifier tid = 0);
				void SendError(Features involved, tj::shared::ExceptionType type, const std::wstring& message);
				void SendListDevices(InstanceID to, tj::shared::ref<Transaction> ti);
				void SendListDevicesReply(const DeviceIdentifier& di, const std::wstring& friendly, TransactionIdentifier ti, in_addr to, bool isLast);
				void SendListPatches(InstanceID to, tj::shared::ref<Transaction> ti);
				void SendListPatchesReply(const PatchIdentifier& pi, const DeviceIdentifier& di, TransactionIdentifier ti, in_addr to, bool isLast);
				void SendResetAll();
				void SendSetPatch(tj::shared::ref<BasicClient> c, const PatchIdentifier& pi, const DeviceIdentifier& di);
				void SendSetClientAddress(tj::shared::ref<BasicClient> c, std::wstring newAddress);
				void SendInput(const PatchIdentifier& patch, const ChannelID& cid, const SubChannelID& scid, float value);
				void SendOutletChange(Channel ch, const std::wstring& outletName, const tj::shared::Any& value);

				void Send(tj::shared::ref<Message> s);
				void Send(tj::shared::ref<Packet> p);
				int GetPort() const;
				std::wstring GetAddress() const;
				int GetBytesSent() const;
				int GetBytesReceived() const;
				unsigned int GetActiveTransactionCount() const;
				void CleanTransactions();

				// Called by network implementation layer, do not call by yourself
				void Receive();
			
			private:
				void Send(tj::shared::ref<Message> s, const sockaddr_in* address);
				void Send(tj::shared::ref<Packet> p, const sockaddr_in* address);
				
				static NetworkInitializer _initializer;

				HWND _window;
				SOCKET _server;
				SOCKET _client;
				char* _recieveBuffer;
				char* _bcastAddress;
				int _port;
				int _bytesSent;
				int _bytesReceived;
				TransactionIdentifier _transactionCounter;
				tj::shared::weak<Node> _network;
				std::map<TransactionIdentifier, tj::shared::ref<Transaction> > _transactions;
				mutable tj::shared::CriticalSection _lock;
		};
	}
}

#pragma pack(pop)
#endif