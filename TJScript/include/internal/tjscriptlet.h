#ifndef _TJSCRIPTLET_H
#define _TJSCRIPTLET_H

namespace tj {
	namespace script {
		class VM;
		typedef unsigned int LiteralIdentifier;

		class Scriptlet {
			friend class VM;
			static const int KInitialScriptletSize = 256;

			public:
				inline Scriptlet(ScriptletType type) {
					_type = type;
					_used = 0;
					Enlarge(KInitialScriptletSize);
				}

				virtual ~Scriptlet() {
					delete[] _code;
				}

				inline bool IsFunction() const {
					return _type==ScriptletFunction;
				}

				inline bool IsLoop() const {
					return _type==ScriptletLoop;
				}

				inline bool IsEmpty() const {
					return _used==0;
				}

				inline const char* GetCode() const {
					return _code;
				}

				template<typename T> inline Scriptlet& Add(const T& x) {
					unsigned int size = sizeof(T)/sizeof(char);
					if(_used+size>_size) {
						Enlarge(2*_size); // double the buffer
					}

					T* tp = (T*)&(_code[_used]);
					*tp = x;
					_used += size;
					return *this;
				}

				inline void AddInstruction(Ops::Codes code) {
					Add<int>((int)code);
				}

				inline unsigned int GetBufferSize() const {
					return _size;
				}

				inline unsigned int GetCodeSize() const {
					return _used;
				}

				template<typename T> inline T Get(unsigned int& pc) const {
					unsigned int size = sizeof(T)/sizeof(char);
					if((pc+(size-1))>_used) Throw(L"Array index out of bounds in scriptlet code reader", tj::shared::ExceptionTypeError);

					T* tp = (T*)&(_code[pc]);
					pc += size;
					return *tp;
				}

				inline ref<Scriptable> GetLiteral(LiteralIdentifier i) {
					if(i>=_literals.size()) Throw(L"Literal not found:"+tj::shared::Stringify(i), tj::shared::ExceptionTypeError);
					return _literals.at(i);
				}

				inline LiteralIdentifier StoreLiteral(ref<Scriptable> s) {
					_literals.push_back(s);
					return ((unsigned int)_literals.size())-1;
				}

			protected:
				inline void Enlarge(unsigned int to) {
					assert(to>=_used);

					char* buffer = new char[to];
					if(_used>0) {
						// copy from old to new buffer
						for(unsigned int a=0;a<_used;++a) {
							buffer[a] = _code[a];
						}

						// delete old buffer
						delete[] _code;
					}

					_code = buffer;
					_size = to;
				}
				
				char* _code;
				unsigned int _size; // size of the code buffer in bytes
				unsigned int _used; // used number of bytes
				ScriptletType _type;
				std::vector< ref<Scriptable> > _literals;
		};

		template<> Scriptlet& Scriptlet::Add(const std::wstring& x);
		template<> wchar_t* Scriptlet::Get(unsigned int& position) const;
	}
}

#endif