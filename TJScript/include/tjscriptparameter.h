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
				inline RequiredParameter(tj::shared::ref<ParameterList> p, std::wstring name, T defaultValue) {
					if(!p) {
						throw ParameterException(name);
					}
					
					if(p->find(name)==p->end()) {
						throw ParameterException(name);
					}

					tj::shared::ref<Scriptable> v = (*(p->find(name))).second;
					if(v==0) {
						throw ParameterException(name);
					}

					value = ScriptContext::GetValue<T>(v, defaultValue);
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
				inline OptionalParameter(tj::shared::ref<ParameterList> p, std::wstring name, T defaultValue) {
					if(p) {
						value = defaultValue;
					}
					else {
						if(p->find(name)==p->end()) {
							value = defaultValue;
						}
						else {
							tj::shared::ref<Scriptable> v = (*(p->find(name))).second;
							if(v==0) {
								value = defaultValue;
							}
							else {
								value = ScriptContext::GetValue<T>(v, defaultValue);
							}
						}
					}
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