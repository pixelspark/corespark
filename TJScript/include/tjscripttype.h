#ifndef _TJSCRIPTTYPE_H
#define _TJSCRIPTTYPE_H

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptType: public Scriptable {
			public:
				virtual ~ScriptType();
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p) = 0;
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p); // for static methods of this type
		};
		
		class SCRIPT_EXPORTED ScriptPackage: public virtual tj::shared::Object {
			public:
				ScriptPackage();
				virtual ~ScriptPackage();
				static tj::shared::strong<ScriptPackage> DefaultInstance();
				virtual void AddType(const std::wstring& ident, tj::shared::strong<ScriptType> st);
				virtual tj::shared::ref<ScriptType> GetType(const std::wstring& type);
			
			protected:
				tj::shared::CriticalSection _lock;
				std::map< std::wstring, tj::shared::ref<ScriptType> > _types;
				static tj::shared::ref<ScriptPackage> _instance;
			
		};
	}
}

#endif