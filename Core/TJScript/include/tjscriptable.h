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
				virtual ~Scriptable();

				/* Returns 0 if command is unknown or used incorrectly. When null value is needed in the script
				return a ScriptNull instance. */
				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params) = 0;

				/* Sets a field; return false if you are not mutable */
				virtual bool Set(Field field, tj::shared::ref<Scriptable> value);
		};
	}
}

#endif