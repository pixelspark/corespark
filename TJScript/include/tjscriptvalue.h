#ifndef _TJSCRIPTVALUE_H
#define _TJSCRIPTVALUE_H
namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptConstantsInitializer {
			public:
				ScriptConstantsInitializer();
		};

		class ScriptConstants {
			public:
				static SCRIPT_EXPORTED tj::shared::ref<Scriptable> True;
				static SCRIPT_EXPORTED tj::shared::ref<Scriptable> False;
				static SCRIPT_EXPORTED tj::shared::ref<Scriptable> Null;

			private:
				static ScriptConstantsInitializer _init;
		};

		template<typename T> class ScriptValue: public Scriptable {
			public:
				ScriptValue(const T& value): _value(value) {
				}

				virtual ~ScriptValue() {
				}

				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params) {
					tj::shared::ref<Scriptable> result = TypeSpecificExecute(command,params);
					if(result) return result;

					if(command==L"toString") {
						return tj::shared::GC::Hold(new ScriptValue<std::wstring>(tj::shared::Stringify(_value)));
					}
					else if(command==L"class") {
						return tj::shared::GC::Hold(new ScriptValue<std::wstring>(Wcs(typeid(T).name())));
					}
					return 0;
				}

				tj::shared::ref<Scriptable> TypeSpecificExecute(Command command, tj::shared::ref<ParameterList> params) {
					return 0;
				}

				virtual T& GetValue() {
					return _value;
				}

			protected:
				T _value;
		};

		/** Type specific script functions **/
		template<> tj::shared::ref<Scriptable> SCRIPT_EXPORTED ScriptValue<std::wstring>::TypeSpecificExecute(tj::script::Command command, tj::shared::ref<ParameterList> params);

		/** Standard typedefs */
		typedef ScriptValue<std::wstring> ScriptString;
		typedef ScriptValue<int> ScriptInt;
		typedef ScriptValue<bool> ScriptBool;
		typedef ScriptValue<double> ScriptDouble;
		
		/** Null type/value **/
		class SCRIPT_EXPORTED ScriptNull: public Scriptable {				
			public:
				ScriptNull();
				virtual ~ScriptNull();
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p);
		};
	}
}

#endif