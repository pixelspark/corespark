#ifndef _TJSCRIPTRANGE_H
#define _TJSCRIPTRANGE_H

namespace tj {
	namespace script {

		class ScriptRangeType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptRangeType();
		};

		class SCRIPT_EXPORTED ScriptRange: public ScriptObject<ScriptRange> {
			friend class ScriptArrayIterator;

			public:	
				static void Initialize();
				ScriptRange(int a, int b);
				virtual ~ScriptRange();


				virtual tj::shared::ref<Scriptable> ToString(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Next(tj::shared::ref<ParameterList> p);

				int _a, _b;
		};
	}
}

#endif