#ifndef _TJSCRIPTARRAY_H
#define _TJSCRIPTARRAY_H

namespace tj {
	namespace script {

		class ScriptArrayType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptArrayType();
		};

		class ScriptArray: public virtual tj::shared::Object, public Scriptable {
			friend class ScriptArrayIterator;

			public:	
				ScriptArray();
				virtual ~ScriptArray();
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p);
				void Set(tj::shared::ref<ParameterList> p);

			protected:
				std::map<std::wstring, tj::shared::ref<Scriptable> > _array;
		};
	}
}

#endif