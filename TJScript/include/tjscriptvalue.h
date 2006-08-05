#ifndef _TJSCRIPTVALUE_H
#define _TJSCRIPTVALUE_H
namespace tj {
	namespace script {

		template<typename T> class ScriptValue: public Scriptable {
			public:
				ScriptValue(T value): _value(value) {
				}

				virtual ~ScriptValue() {
				}

				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params) {
					if(command==L"toString") {
						return tj::shared::GC::Hold(new ScriptValue<std::wstring>(tj::shared::Stringify(_value)));
					}
					return 0;
				}

				virtual T& GetValue() {
					return _value;
				}

			protected:
				T _value;
		};

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