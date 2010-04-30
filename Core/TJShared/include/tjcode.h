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
 
 #ifndef _TJCODE_H
#define _TJCODE_H

#include "tjsharedinternal.h"
#include "tjvector.h"

#pragma pack(push,1)

namespace tj {
	namespace shared {
		class EXPORTED Data: public virtual Object {
			public:
				virtual ~Data();
				virtual Bytes GetSize() const = 0;
				virtual const char* GetBuffer() const = 0;
				virtual char* TakeOverBuffer(bool clearMine = true) = 0;
		};

		class EXPORTED DataReader: public Data {
			public:
				DataReader(const char* code, Bytes size);
				virtual ~DataReader();
				virtual Bytes GetSize() const;
				virtual const char* GetBuffer() const;
				virtual char* TakeOverBuffer(bool clearMine);

				template<typename T> T Get(unsigned int& position) {
					unsigned int size = sizeof(T)/sizeof(char);
					if((position+(size-1))>_size) Throw(L"Array index out of bounds in code reader", ExceptionTypeError);

					T* tp = (T*)&(_code[position]);
					position += size;
					return *tp;
				}

			protected:
				char* _code;
				Bytes _size;
		};

		template<> EXPORTED String DataReader::Get(unsigned int& position);
		template<> EXPORTED tj::shared::Vector DataReader::Get(unsigned int& position);

		class EXPORTED DataWriter: public Data {
			public:
				DataWriter(Bytes initialSize = 1024);
				virtual ~DataWriter();
				virtual Bytes GetSize() const;
				virtual Bytes GetCapacity() const;

				template<typename T> DataWriter& Add(const T& x) {
					if(_buffer==0) {
						Throw(L"DataWriter written to after buffer has been taken over!", ExceptionTypeSevere);
					}

					unsigned int size = sizeof(T)/sizeof(char);
					Grow(size);

					T* tp = (T*)&(_buffer[_pos]);
					*tp = x;
					_pos += size;
					return *this;
				}

				inline void Append(const char* buffer, Bytes size) {
					if(_buffer==0) {
						Throw(L"DataWriter appended to after buffer has been taken over!", ExceptionTypeSevere);
					}

					Grow(size*sizeof(char));
					for(unsigned int a=0;a<size;a++) {
						_buffer[_pos+a] = buffer[a];
					}
					_pos += size;
				}

				virtual const char* GetBuffer() const;
				virtual char* TakeOverBuffer(bool clearMine);

				inline void Reset() {
					_pos = 0;
				}
				
			protected:
				void Grow(Bytes size);

				Bytes _size;
				Bytes _pos;

			public:
				char* _buffer;
		};

		template<> EXPORTED DataWriter& DataWriter::Add(const String& x);
	}
}

#pragma pack(pop)
#endif