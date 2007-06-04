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
		template<> tj::shared::ref<Scriptable> ScriptValue<std::wstring>::TypeSpecificExecute(tj::script::Command command, tj::shared::ref<ParameterList> params) {
			if(command==L"length") {
				return tj::shared::GC::Hold(new ScriptValue<int>((int)_value.length()));
			}
			else if(command==L"toUpper") {
				std::wstring temp = _value;
				std::transform(temp.begin(), temp.end(), temp.begin(), toupper);
				return tj::shared::GC::Hold(new ScriptValue<std::wstring>(temp));
			}
			else if(command==L"toLower") {
				std::wstring temp = _value;
				std::transform(temp.begin(), temp.end(), temp.begin(), tolower);
				return tj::shared::GC::Hold(new ScriptValue<std::wstring>(temp));
			}
			return 0;
		}


		/** Standard typedefs */
		typedef ScriptValue<std::wstring> ScriptString;
		typedef ScriptValue<int> ScriptInt;
		typedef ScriptValue<bool> ScriptBool;
		typedef ScriptValue<double> ScriptDouble;

		struct _Null {
			_Null() {
			}
		};

		class SCRIPT_EXPORTED ScriptNull: public ScriptValue<_Null> {				
			public:
				ScriptNull(): ScriptValue<_Null>(_Null()) {
				}
				
				virtual ~ScriptNull() {
				}
		};
	}
}

SCRIPT_EXPORTED std::wostringstream& operator <<(std::wostringstream& i, const tj::script::_Null& n);

#endif