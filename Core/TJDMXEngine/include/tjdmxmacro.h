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
 
 #ifndef _TJDMXMACRO_H
#define _TJDMXMACRO_H

#include "tjdmxinternal.h"
#include "tjdmxcontroller.h"

namespace tj {
	namespace dmx {
		using namespace tj::shared;

		/** Simple DMX macro that changes a single, 8-bit DMX channel value. It is not tracked by
		DMXController, as is the case for 'complex' DMX macros **/
		class DMX_EXPORTED SimpleDMXMacro: public DMXMacro {
			public:
				SimpleDMXMacro(DMXController* controller, int channel, DMXSource src, bool invert = false);
				virtual ~SimpleDMXMacro();
				virtual void Set(float value);
				virtual float Get() const;
				virtual std::wstring GetAddress() const;
				virtual float GetResult() const;
				virtual float GetResultCached() const;
				virtual DMXMacroType GetType() const;

			protected:
				DMXController* _controller;
				DMXSlot _channel;
				DMXSource _source;
				bool _invert;
				std::wstring _address;
		};

		/** Simple 'null' DMX macro which just discards any changes. **/
		class DMX_EXPORTED NullDMXMacro: public DMXMacro {
			public:
				NullDMXMacro();
				virtual ~NullDMXMacro();
				virtual void Set(float value);
				virtual float Get() const;
				virtual DMXMacroType GetType() const;
				virtual std::wstring GetAddress() const;
				virtual float GetResult() const;

			protected:
				float _value;
		};

		/** 'Precise' DMX value which controls two DMX channels (coarse and fine) to send a 16-bit value. Not tracked
		by DMXController **/
		class DMX_EXPORTED PreciseDMXMacro: public DMXMacro {
			public:
				PreciseDMXMacro(DMXController* controller, int channel, DMXSource src, bool invert = false);
				virtual ~PreciseDMXMacro();
				virtual void Set(float value);
				virtual float Get() const;
				virtual std::wstring GetAddress() const;
				virtual float GetResult() const;
				virtual DMXMacroType GetType() const;

			protected:
				void Set(int v);
				DMXController* _controller;
				bool _invert;
				int _channel;
				DMXSource _source;
		};

		class DMX_EXPORTED ComplexDMXMacro: public DMXMacro {
			public:
				ComplexDMXMacro(DMXController* controller, std::wstring address, DMXSource src, bool invert = false);
				virtual ~ComplexDMXMacro();
				virtual void Set(float value);
				virtual float Get() const;
				virtual DMXMacroType GetType() const;
				virtual std::wstring GetAddress() const;
				virtual float GetResult() const;

			protected:
				DMXSource _source;
				std::wstring _address;
				std::vector<int> _channels;
				DMXController* _controller;
				float _value;
				bool _invert;
		};
	}
}

#endif