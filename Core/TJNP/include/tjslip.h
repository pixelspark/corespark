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
 
 #ifndef _TJ_SLIP_H
#define _TJ_SLIP_H

#include "tjnpinternal.h"
#pragma warning(push)
#pragma warning(disable: 4251 4275)

namespace tj {
	namespace np {
		class NP_EXPORTED SLIPFrameDecoder: public virtual tj::shared::Object {
			public:
				SLIPFrameDecoder();
				virtual ~SLIPFrameDecoder();
				virtual void Append(const unsigned char* data, tj::shared::Bytes length);
				static void EncodeSLIPFrame(const unsigned char* data, tj::shared::Bytes length, tj::shared::strong<tj::shared::DataWriter> cw);

			protected:
				virtual void OnPacketReceived(const unsigned char* data, tj::shared::Bytes length);
				
				tj::shared::ref<tj::shared::DataWriter> _buffer;

			private:
				bool _isReceivingPacket;
				bool _isDiscardingPacket;
				bool _lastCharacterWasEscape;
				const static unsigned char KSLIPEndCharacter;
				const static unsigned char KSLIPEscapeCharacter;
				const static unsigned char KSLIPEscapeEscapeCharacter;
				const static unsigned char KSLIPEscapeEndCharacter;
		};
		
		class NP_EXPORTED QueueSLIPFrameDecoder: public SLIPFrameDecoder {
			public:
				QueueSLIPFrameDecoder();
				virtual ~QueueSLIPFrameDecoder();
				tj::shared::ref<tj::shared::DataReader> NextPacket();
				
			protected:
				virtual void OnPacketReceived(const unsigned char* data, tj::shared::Bytes length);
				std::deque< tj::shared::ref<tj::shared::DataReader> > _buffers;
		};
	}
}

#pragma warning(pop)
#endif