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
 
 #ifndef _TJSCRIPTRANGE_H
#define _TJSCRIPTRANGE_H

#pragma warning(push)
#pragma warning(disable: 4251 4275)

namespace tj {
	namespace script {

		class ScriptRangeType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptRangeType();
		};

		// ScriptRange is [a,b]
		class SCRIPT_EXPORTED ScriptRange: public ScriptObject<ScriptRange> {
			friend class ScriptArrayIterator;

			public:	
				static void Initialize();
				ScriptRange(int a, int b);
				virtual ~ScriptRange();


				virtual tj::shared::ref<Scriptable> ToString(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Next(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> IsInside(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Size(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> From(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> To(tj::shared::ref<ParameterList> p);

				int _a, _b;
		};
	}
}

#pragma warning(pop)

#endif