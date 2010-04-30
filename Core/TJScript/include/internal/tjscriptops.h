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
 
 #ifndef _TJSCRIPTOPS_H
#define _TJSCRIPTOPS_H

namespace tj {
	namespace script {
		class VM;

		class Ops {
			public:
				typedef void (*OpHandler)(VM* vm);

				/* Below are all opcodes. The comments give an indication of what
				the ops do. Everything between [brackets] are stack values,
				between <angle brackets> are read from the scriptlet code */
				enum Code {
					OpNop = 0,
					OpPushString,			// [..] => [ScriptString]
					OpPushDouble,			// [..] => [ScriptDouble]
					OpPushTrue,				// [..] => [true]
					OpPushFalse,			// [..] => [false]
					OpPushInt,				// [..] => [ScriptInt]
					OpPushNull,				// [..] => [null]
					OpPop,					// [any] => [..]
					OpCall,					// [parameters] [callee] [function name] => [result]
					OpCallGlobal,			// [parameters] [function name] => [result]
					OpNew,					// [typename] [parameters] => [instance]
					OpSave,					// [name] [value] => [..]
					OpEquals,				// [a] [b] => [a==b]
					OpNegate,				// [a] => [-a]
					OpAdd,					// [a] [b] => [a+b]
					OpSub,					// [a] [b] => [a-b]
					OpMul,					// [a] [b] => [a*b]
					OpDiv,					// [a] [b] => [a/b]
					OpAnd,					// [a] [b] => [a && b]
					OpOr,					// [a] [b] => [a || b]
					OpBranchIf,				// [bool] <int id> => [..]
					OpParameter,			// [parameterlist] [key] [value] => [parameterlist]
					OpNamelessParameter,	// [parameterlist] [value] => [parameterlist]
					OpPushParameter,		// [..] => [parameterlist]
					OpLoadScriptlet,		// <int id> => [scriptable]
					OpReturn,				
					OpReturnValue,			
					OpGreaterThan,			// [a] [b] => [a>b]
					OpLessThan,				// [a] [b] => [a<b]
					OpXor,					// [a] [b] => [(a && !b) || (!a && b)]
					OpBreak,				
					OpIndex,				// [varname] [index] => [value]
					OpIterate,				// [varname] [iterable] => [..]
					OpPushDelegate,
					OpSetField,				// [scriptable] [identifier] [value] => [value]
					OpAddToArray,			// [array] [scriptable] => [array]
					OpPushArray,			// [..] => [array]
					OpType,					// [string] => [type scriptable]
					OpEndScriptlet,			// Same as OpReturn but doesn't loop until it exits a function up the stack
					_OpLast,				// Should always be the last op
				};

				static const wchar_t* Names[_OpLast];
				static OpHandler Handlers[_OpLast];
		};
	}
}

#endif