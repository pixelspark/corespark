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
 
 #ifndef _TJSCRIPT_DELEGATE_H
#define _TJSCRIPT_DELEGATE_H

namespace tj {
	namespace script {
		using tj::shared::ref;
		using tj::shared::strong;

		class SCRIPT_EXPORTED ScriptDelegate: public ScriptObject<ScriptDelegate> {
			public:
				ScriptDelegate(ref<CompiledScript> sc, ref<ScriptContext> ctx);
				virtual ~ScriptDelegate();
				static void Initialize();
				ref<CompiledScript> GetScript();
				ref<ScriptContext> GetContext();
				ref<Scriptable> SToString(ref<ParameterList> p);

			protected:
				ref<CompiledScript> _cs;
				ref<ScriptContext> _context;
		};

		class SCRIPT_EXPORTED ScriptFuture: public ScriptObject<ScriptFuture>, public tj::shared::Future {
			public:
				ScriptFuture(ref<CompiledScript> cs, ref<ScriptContext> originalContext);
				virtual ~ScriptFuture();
				virtual void Run();
				virtual bool IsConcrete();
				virtual ref<Scriptable> GetReturnValue();
				virtual bool WaitForCompletion();
				virtual void AddDependency(Field field, strong<ScriptFuture> fut);
				virtual void AddVariable(Field field, strong<Scriptable> sc);

				static void Initialize();
				virtual ref<Scriptable> SToString(ref<ParameterList> p);

			protected:
				strong<CompiledScript> _cs;
				strong<ScriptContext> _originalContext;
				strong<ScriptScope> _scope;
				ref<Scriptable> _returnValue;
				bool _isConcrete;
				tj::shared::Event _finished;
		};
	}
}

#endif