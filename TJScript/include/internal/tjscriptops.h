#ifndef _TJSCRIPTOPS_H
#define _TJSCRIPTOPS_H

namespace tj {
	namespace script {
		class Op {
			public:
				virtual void Execute(tj::shared::ref<VM> vm) = 0;
				virtual inline std::wstring GetName() = 0;
		};

		// pushes a value on the stack
		// ... => [Scriptable]
		class  OpPush: public Op {
			public:
				OpPush(tj::shared::ref<Scriptable> s);
				virtual ~OpPush() {};
				virtual void Execute(tj::shared::ref<VM> stack);
				virtual std::wstring GetName();

			protected:
				tj::shared::ref<Scriptable> _value;
		};

		// removes the most recently pushed object from the stack
		// [any] => ...
		class  OpPop: public Op {
			public:
				virtual ~OpPop() {};
				virtual void Execute(tj::shared::ref<VM> stack);
				virtual inline std::wstring GetName() {return L"OpPop";}
		};

		// takes a key and value object and adds it to a parameter list
		// [parameterlist] [key] [value] => [parameterlist]
		class  OpParameter: public Op {
			public:
				virtual ~OpParameter() {};
				virtual void Execute(tj::shared::ref<VM> stack);
				virtual inline std::wstring GetName() {return L"OpParameter";}
		};

		// takes a value object and adds it to a parameter list (first parameter with key="1", second "2" etc
		// [parameterlist] [value] => [parameterlist]
		class OpNamelessParameter: public Op {
			public:
				virtual ~OpNamelessParameter() {}
				virtual void Execute(tj::shared::ref<VM> stack);
				virtual inline std::wstring GetName() { return L"OpNamelessParameter"; };
		};

		// creates a parameterlist on the stack
		// ... => [parameterlist]
		class  OpPushParameter: public Op {
			public:
				virtual ~OpPushParameter() {};
				virtual void Execute(tj::shared::ref<VM> stack);
				virtual inline std::wstring GetName() {return L"OpPushParameter";}
		};

		// calls a native function
		// [callee] [function name] => [result]
		class  OpCall: public Op {
			public:
				virtual ~OpCall() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpCall";}
		};

		// calls a native function using the global scope as callee
		// [function name] => [result]
		class  OpCallGlobal: public Op {
			public:
				virtual ~OpCallGlobal() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpCallGlobal";}
		};

		// creates a (probably native) type instance
		// [typename] [parameterlist] => [instance of type]
		class  OpNew: public Op {
			public:
				virtual ~OpNew() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpNew";}
		};

		// saves a variable in the current scope
		// [name] [value] => ...
		class  OpSave: public Op {
			public:
				virtual ~OpSave() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpSave";}
		};

		// tests if two objects are equal. Pushes the result as ScriptBool on the stack
		// [object a] [object b] => [ScriptBool]
		class  OpEquals: public Op {
			public:
				virtual ~OpEquals() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpEquals";}
		};

		// negates or inverts ScriptBool, ScriptInt and ScriptDouble
		// [value] => [-value]
		class  OpNegate: public Op {
			public:
				virtual ~OpNegate() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpNegate";}
		};

		// Adds two values
		// [int/double] [int/double] => [result]
		class  OpAdd: public Op {
			public:
				virtual ~OpAdd() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpAdd";}
		};

		class  OpSub: public Op {
			public:
				virtual ~OpSub() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpSub";}
		};

		class  OpMul: public Op {
			public:
				virtual ~OpMul() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpMul";}
		};

		class  OpDiv: public Op {
			public:
				virtual ~OpDiv() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpDiv";}
		};

		// conditional branch; if the top of the stack is a boolean and it is true,
		// the specified scriptlet (by index) is called.
		// [bool] => ...
		class OpBranchIf: public Op {
			public:
				OpBranchIf(int scriptlet);
				virtual ~OpBranchIf() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpBranchIf";}

			protected:
				int _scriptlet;
		};

		// loads a scriptlet as scriptable on the stack
		// ... => [scriptable]
		class OpLoadScriptlet: public Op {
			public:
				OpLoadScriptlet(int scriptlet);
				virtual ~OpLoadScriptlet() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpLoadScriptlet";}

			protected:
				int _scriptlet;
		};

		// returns from a scriptlet, not leaving any return value (so, giving null back implicitly)
		class OpReturn: public Op {
			public:
				virtual ~OpReturn() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpReturn";}
		};

		// returns from a scriptlet, leaving a return value to the caller
		class OpReturnValue: public Op {
			public:
				virtual ~OpReturnValue() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpReturnValue";}
		};

		// [bool] [bool] => [bool]
		class OpAnd: public Op {
			public:
				virtual ~OpAnd() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpAnd";}
		};

		// [bool] [bool] => [bool]
		class OpOr: public Op {
			public:
				virtual ~OpOr() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpOr";}
		};

		// [number][number] => [bool]
		class OpGreaterThan: public Op {
			public:
				virtual ~OpGreaterThan() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpGreaterThan";}
		};

		
		// [number][number] => [bool]
		class OpLessThan: public Op {
			public:
				virtual ~OpLessThan() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpLessThan";}
		};


		// [bool] [bool] => [bool]
		class OpXor: public Op {
			public:
				virtual ~OpXor() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpXor";}
		};

		// OpBreak: quit the current scriptlet and resume the calling scriptlet
		class OpBreak: public Op {
			public:
				virtual ~OpBreak() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpBreak";}
		};


		// OpIndex: used for array indices; array[index] 
		// [varname] [index key] => [value or null]
		// calls get(key=...) on object
		class OpIndex: public Op {
			public:
				virtual ~OpIndex() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpIndex";}
		};

		// OpIterate: executes a scriptlet for each value in a container saving that value
		// to a specific variable
		// [varname] [iterable] => ...
		class OpIterate: public Op {
			public:
				inline OpIterate(int scriptlet) {
					_scriptlet = scriptlet;
				}

				virtual ~OpIterate() {};
				virtual void Execute(tj::shared::ref<VM> vm);
				virtual inline std::wstring GetName() {return L"OpIterate";}

				int _scriptlet;
		};


	}
}

#endif