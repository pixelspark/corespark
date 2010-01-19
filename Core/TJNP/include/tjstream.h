#ifndef _TJ_NP_STREAM_H
#define _TJ_NP_STREAM_H

#include "tjnpinternal.h"
#include "tjprotocol.h"
#include "tjtransaction.h"

#pragma pack(push,1)
#pragma warning(push)
#pragma warning(disable: 4251 4275)

namespace tj {
	namespace np {
		class Packet;

		/** Message represents a message sent from master to client through a Stream. It can contain
		any data you want. **/
		class NP_EXPORTED Message: public virtual tj::shared::Object {
			friend class Socket; // for buffer read access
			friend class Stream;

			public:
				Message(PacketAction ac, TransactionIdentifier ti = 0);
				Message(bool toPlugin, const GroupID& group, const Channel& channel, const PluginHash& ph);
				virtual ~Message();
				bool IsSent() const;

				template<typename T> void Add(const T& x) {
					if(_sent) return;
					_writer->Add<T>(x);

					// update header
					_header->_size += sizeof(T)/sizeof(char);
				}

				unsigned int GetSize();
				PacketHeader* GetHeader();
				void SetSent();

				inline void SetSendToPlugin(bool t) {
					_header->_action = (t ? ActionUpdatePlugin : ActionUpdate);
				}

				tj::shared::strong<Packet> ConvertToPacket();

			protected:
				const char* GetBuffer();
				tj::shared::strong<tj::shared::DataWriter> _writer;
				PacketHeader* _header;
				bool _sent;
		};

		template<> inline void Message::Add(const std::wstring& x) {
			if(_sent) return;
			_writer->Add(x);

			// update header
			_header->_size += (unsigned short)(((unsigned int)x.length())*sizeof(wchar_t));
			_header->_size += sizeof(int);
		}

		template<> inline void Message::Add(const tj::shared::Vector& v) {
			Add<float>(v.x);
			Add<float>(v.y);
			Add<float>(v.z);
		}

		/** The Stream is responsible for passing messages from the master to the client. **/
		class Stream: public virtual tj::shared::Object {
			public:
				virtual ~Stream() {}
				virtual tj::shared::strong<Message> Create() = 0;
				virtual void Send(tj::shared::ref<Message> msg, bool reliable = false) = 0;
		};
	}
}

#pragma warning(pop)
#pragma pack(pop)
#endif