#ifndef _TJSCRIPTTYPE_H
#define _TJSCRIPTTYPE_H

namespace tj {
	namespace script {

		class SCRIPT_EXPORTED ScriptType {
			public:
				virtual ~ScriptType();
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p) = 0;
		};
	}
}

#endif