#ifndef _TJSCRIPTPARAMETER_H
#define _TJSCRIPTPARAMETER_H

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ParameterException: public ScriptException {
			public:
				ParameterException(std::wstring parameter);
				virtual ~ParameterException();
		};

		template<typename T> class RequiredParameter {
			public:
				inline RequiredParameter(tj::shared::ref<ParameterList> p, std::wstring name, T defaultValue, int namelessIndex=-1) {
					if(!p) {
						throw ParameterException(name);
					}
					
					tj::shared::ref<Scriptable> v;
					if(p->find(name)!=p->end()) {
						v = (*(p->find(name))).second;
					}
					else if(namelessIndex>=0 && p->find(Stringify(namelessIndex))!=p->end()) {
						v = (*p->find(Stringify(namelessIndex))).second;
					}
					
					if(v==0) {
						throw ParameterException(name);
					}

					value = ScriptContext::GetValue<T>(v, defaultValue);
				}

				inline ~RequiredParameter() {
				}

				inline operator T&() {
					return value;
				}

				inline T& Get() {
					return value;
				}

				T value;
		};

		template<typename T> class OptionalParameter {
			public:
				inline OptionalParameter(tj::shared::ref<ParameterList> p, std::wstring name, T defaultValue, int namelessIndex=-1) {
					if(p) {
						tj::shared::ref<Scriptable> v;
						if(p->find(name)!=p->end()) {
							v = (*(p->find(name))).second;
						}
						else if(namelessIndex>=0 && p->find(Stringify(namelessIndex))!=p->end()) {
							v = (*(p->find(Stringify(namelessIndex)))).second;
						}

						if(v) {
							value = ScriptContext::GetValue<T>(v, defaultValue);
							return;
						}
					}

					value = defaultValue;
				}

				inline ~OptionalParameter() {
				}

				inline operator T&() {
					return value;
				}

				inline T& Get() {
					return value;
				}

				T value;
		};
	}
}

#endif