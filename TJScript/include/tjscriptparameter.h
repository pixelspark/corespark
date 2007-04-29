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
					
					tj::shared::ref<Scriptable> v = p->Get(name);
					if(!v) {
						if(namelessIndex>=0) {
							std::wstring namelessKey = Stringify(namelessIndex);
							v = p->Get(namelessKey);

							if(!v) {
								throw ParameterException(name);
							}
						}
						else {
							throw ParameterException(name);
						}
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
						v = p->Get(name);
						
						if(!v && namelessIndex>=0) {
							std::wstring namelessKey = Stringify(namelessIndex);
							v = p->Get(namelessKey);
						}
						
						if(!v) {
							v = GC::Hold(new ScriptValue<T>(defaultValue));
						}

						value = v?ScriptContext::GetValue<T>(v, defaultValue):defaultValue;
					}
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