#ifndef _TJSCRIPTABLE_H
#define _TJSCRIPTABLE_H

namespace tj {
	namespace script {
		class Scriptable; 

		typedef std::map< std::wstring, tj::shared::ref<Scriptable> > ParameterList;
		typedef std::wstring CommandType;
		typedef const CommandType& Command;

		/** This is the most basic scripting interface that any object used in the scripting engine should support.
		More advanced objects can better be derived from ScriptObject if they want to provide advanced stuff
		like reflection. **/
		class Scriptable {
			public:
				/* Returns 0 if command is unknown or used incorrectly. When null value is needed in the script
				return a ScriptNull instance. */
				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params) = 0;
		};
	}
}

#endif