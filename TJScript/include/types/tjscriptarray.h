#ifndef _TJSCRIPTARRAY_H
#define _TJSCRIPTARRAY_H

namespace tj {
	namespace script {

		class ScriptArrayType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptArrayType();
		};

		class SCRIPT_EXPORTED ScriptArray: public virtual tj::shared::Object, public Scriptable {
			friend class ScriptArrayIterator;

			public:	
				ScriptArray();
				virtual ~ScriptArray();
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p);
				void Set(tj::shared::ref<ParameterList> p);
				std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator GetBegin();
				std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator GetEnd();

			protected:
				std::map<std::wstring, tj::shared::ref<Scriptable> > _array;
				tj::shared::CriticalSection _lock;
		};
	}
}

#endif