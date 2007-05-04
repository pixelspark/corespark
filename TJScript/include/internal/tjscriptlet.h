#ifndef _TJSCRIPTLET_H
#define _TJSCRIPTLET_H

namespace tj {
	namespace script {
		class VM;

		class Op {
			public:
				virtual void Execute(tj::shared::ref<VM> vm) = 0;
				virtual inline std::wstring GetName() = 0;
		};

		class Scriptlet {
			friend class VM;

			public:
				inline Scriptlet(ScriptletType type) {
					_type = type;
				}

				virtual ~Scriptlet() {
				}

				inline void Add(tj::shared::ref<Op> op) {
					_code.push_back(op);
				}

				inline bool IsFunction() const {
					return _type==ScriptletFunction;
				}

				inline bool IsLoop() const {
					return _type==ScriptletLoop;
				}

				inline bool IsEmpty() const {
					return _code.size()==0;
				}

			protected:
				std::vector< tj::shared::ref<Op> > _code;
				ScriptletType _type;
		};
	}
}

#endif