#ifndef _TJ_SLIP_H
#define _TJ_SLIP_H

#include "internal/tjnp.h"

namespace tj {
	namespace np {
		class NP_EXPORTED SLIPFrameDecoder: public virtual tj::shared::Object {
			public:
				SLIPFrameDecoder();
				virtual ~SLIPFrameDecoder();
				virtual void Append(const unsigned char* data, unsigned int length);

			protected:
				virtual void OnPacketReceived(const unsigned char* data, unsigned int length);
				
				tj::shared::ref<tj::shared::CodeWriter> _buffer;

			private:
				bool _isReceivingPacket;
				bool _lastCharacterWasEscape;
				const static unsigned char KSLIPEndCharacter;
				const static unsigned char KSLIPEscapeCharacter;
				const static unsigned char KSLIPEscapeEscapeCharacter;
				const static unsigned char KSLIPEscapeEndCharacter;
		};
	}
}

#endif