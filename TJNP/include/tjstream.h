#ifndef _TJ_NP_STREAM_H
#define _TJ_NP_STREAM_H

#pragma pack(push,1)

namespace tj {
	namespace np {
		
		/** Message represents a message sent from master to client through a Stream. It can contain
		any data you want. **/
		class NP_EXPORTED Message: public virtual tj::shared::Object {
			friend class Socket; // for buffer read access
			friend class Stream;

			public:
				Message(bool toPlugin);
				virtual ~Message();
				bool IsSent();

				template<typename T> void Add(const T& x) {
					if(_sent) return;
					_writer->Add<T>(x);

					// update header
					_header->_size += sizeof(T)/sizeof(char);
				}

				void SetChannel(Channel id);
				void SetPlugin(PluginHash pt);
				unsigned int GetSize();
				PacketHeader* GetHeader();
				void SetSent();

				/* Messages can either be sent to the stream player or the plugin
				on the client. Use this to set the target (default is streamplayer for
				TrackStream, plugin for LiveStream) */
				inline void SetSendToPlugin(bool p) {
					_toPlugin = p;
				}

				bool IsSentToPlugin() const;

			protected:
				const char* GetBuffer();
				tj::shared::ref<tj::shared::CodeWriter> CreateWriter();
				tj::shared::ref<tj::shared::CodeWriter> _writer;
				PacketHeader* _header;
				bool _sent;
				bool _toPlugin;
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
				virtual tj::shared::ref<Message> Create() = 0;
				virtual void Send(tj::shared::ref<Message> msg) = 0;
		};
	}
}

#pragma pack(pop)
#endif