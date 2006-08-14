#ifndef _TJSCRIPTRANGE_H
#define _TJSCRIPTRANGE_H

namespace tj {
	namespace script {

		class ScriptRangeType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptRangeType();
		};

		class SCRIPT_EXPORTED ScriptRange: public virtual tj::shared::Object, public Scriptable {
			friend class ScriptArrayIterator;

			public:	
				ScriptRange(int a, int b);
				virtual ~ScriptRange();
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p);
		
				int _a, _b;
		};
	}
}

#endif