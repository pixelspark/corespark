#ifndef _SCRIPTITERATOR_H
#define _SCRIPTITERATOR_H

namespace tj {
	namespace script {

		template<typename T> class ScriptIterator: public Scriptable {
			public:
				virtual ~ScriptIterator() {
				}

				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p) {
					if(c==L"toString") {
						return tj::shared::GC::Hold(new ScriptString(L"[Iterator]"));
					}
					else if(c==L"next") {
						return Next();
					}
					return 0;
				}

				virtual tj::shared::ref<Scriptable> Next() = 0;
		};
	}
}

#endif