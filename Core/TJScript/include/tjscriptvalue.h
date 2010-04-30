/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
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
		
		class SCRIPT_EXPORTED ScriptAny: public Scriptable {
			public:
				virtual ~ScriptAny();
				virtual tj::shared::Any Unbox() const = 0;
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> ps);
		};

		/** This is the class that 'wraps' values like strings, doubles, ints & booleans as script objects. Because it 
		 is defined as a template, a few subtleties are important here. The most important caveat is that the ScriptValue
		 class may be compiled for each module individually (i.e. one for TJScript and one for the application using the
		 TJScript library). This is not bad, as long as these classes are of the same version. What does matter is that
		 client applications can never cast a ref<Scriptable> to a ref<ScriptValue<T>>, because the typeinfo of their
		 ScriptValue-'implementation' does not match the typeinfo of the one that TJScript uses. 
		 
		 This is why all conversion ('unboxing') of ScriptValue values happens through ScriptAny::Unbox (using the TJShared
		 Any type). Please do not try to use the ScriptValue<T> from client applications directly, as these problems might occur
		 (especially under GCC; it used to work fine under MSVC++ by the way).
		 **/
		template<typename T> class ScriptValue: public ScriptAny, public tj::shared::Recycleable {
			public:
				ScriptValue(const T& value): _value(value) {
				}
			
				ScriptValue() {
				}

				virtual ~ScriptValue() {
				}
			
				virtual void SetValue(const T& x) {
					_value = x;
				}

				virtual T& GetValue() {
					return _value;
				}
			
				virtual tj::shared::Any Unbox() const {
					return tj::shared::Any(_value);
				}

			protected:
				T _value;
		};

		/** Standard typedefs */
		typedef ScriptValue<std::wstring> ScriptString;
		typedef ScriptValue<int> ScriptInt;
		typedef ScriptValue<bool> ScriptBool;
		typedef ScriptValue<double> ScriptDouble;
		typedef ScriptValue<tj::shared::Any> ScriptAnyValue;
		
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