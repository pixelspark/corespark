#ifndef _TJSCRIPTMATH_H
#define _TJSCRIPTMATH_H

namespace tj {
	namespace script {
		class ScriptMathType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptMathType();
		};

		class SCRIPT_EXPORTED ScriptMath: public virtual tj::shared::Object, public Scriptable {
			public:	
				ScriptMath();
				virtual ~ScriptMath();
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p);
		};
	}
}

#endif