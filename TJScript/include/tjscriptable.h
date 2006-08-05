#ifndef _TJSCRIPTABLE_H
#define _TJSCRIPTABLE_H

namespace tj {
	namespace script {
		class Scriptable; 

		typedef std::map< std::wstring, tj::shared::ref<Scriptable> > ParameterList;
		typedef const std::wstring& Command;

		class Scriptable {
			public:
				/* Returns 0 if command is unknown or used incorrectly. When null value is needed in the script
				return a ScriptNull instance. */
				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params) = 0;
		};

		class SCRIPT_EXPORTED ScriptConstants {
			public:
				static tj::shared::ref<Scriptable> Null();

			protected:
				static tj::shared::ref<Scriptable> _null;
		};
	}
}

#endif