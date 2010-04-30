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
 
 #ifndef _TJSCRIPTMATH_H
#define _TJSCRIPTMATH_H

#pragma warning(push)
#pragma warning(disable: 4251 4275)

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptMath: public ScriptObject<ScriptMath> {
			public:	
				ScriptMath();
				virtual ~ScriptMath();
				static void Initialize();

				// Script methods
				virtual tj::shared::ref<Scriptable> Sin(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Cos(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Tan(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Atan2(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Atan(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Acos(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Asin(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Fmod(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Pow(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Pi(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> E(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Random(tj::shared::ref<ParameterList> p);
		};

		class ScriptMathType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p);
				virtual ~ScriptMathType();

			protected:
				tj::shared::ref<ScriptMath> _instance;
		};
	}
}

#pragma warning(pop)

#endif