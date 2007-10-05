#ifndef _TJSCRIPTABLE_H
#define _TJSCRIPTABLE_H

namespace tj {
	namespace script {
		class Scriptable; 
		class ScriptScope;

		typedef ScriptScope ParameterList;
		typedef std::wstring CommandType;
		typedef const CommandType& Command;
		typedef const std::wstring& Field;

		/** This is the most basic scripting interface that any object used in the scripting engine should support.
		More advanced objects can better be derived from ScriptObject if they want to provide advanced stuff
		like reflection. **/
		class SCRIPT_EXPORTED Scriptable {
			public:
				/* Returns 0 if command is unknown or used incorrectly. When null value is needed in the script
				return a ScriptNull instance. */
				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params) = 0;

				/* Sets a field; return false if you are not mutable */
				virtual bool Set(Field field, tj::shared::ref<Scriptable> value) { return false; };
		};
	}
}

#endif