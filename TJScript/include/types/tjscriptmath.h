#ifndef _TJSCRIPTMATH_H
#define _TJSCRIPTMATH_H

namespace tj {
	namespace script {
		class ScriptMathType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptMathType();
		};

		class SCRIPT_EXPORTED ScriptMath: public ScriptObject<ScriptMath> {
			public:	
				ScriptMath();
				virtual ~ScriptMath();
				static void Initialize();

				// Script methods
				virtual tj::shared::ref<Scriptable> Sin(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Cos(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Tan(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Atan2(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Atan(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Acos(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Asin(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Fmod(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Pow(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Pi(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Random(tj::shared::ref<ParameterList> p);
		};
	}
}

#endif