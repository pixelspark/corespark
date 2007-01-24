#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

VM::VM(int stackLimit): _stack(stackLimit) {
	_scope = 0;
	_debug = false;
}

VM::~VM() {
}

void VM::SetDebug(bool d) {
	_debug = d;
}

void VM::Call(ref<Scriptlet> s, ref<ScriptParameterList> p) {
	if(s->IsEmpty()) {
		return;
	}
	// set stack size in caller for stack recovery
	if(!_call.empty()) {
		StackFrame& current = GetStackFrame();
		current._stackSize = _stack.GetSize();
	}
	_call.push_back(GC::Hold(new StackFrame(s,0)));

	ScriptScope* ss = new ScriptScope(p);
	ss->SetPrevious(_scope);
	_scope = GC::Hold(ss);
}

void VM::Call(int n) {
	Call(_script->GetScriptlet(n));
}

/** Return is essentially the same as break: it pops the call stack and unchains a scope (destroying
variables created inside the scriptlet). The difference is that return pops the call stack until a
function scriptlet is encountered, and Break does the same thing until a loop scriptlet is encountered
and quits that loop too. **/
void VM::Return(bool returnValue) {
	while(true) {
		bool isFunction = GetStackFrame()->_scriptlet->IsFunction();
		_call.pop_back();
		_scope = _scope->GetPrevious();
		if(isFunction) {
			break;
		}
	}

	ref<Scriptable> returnedValue = _stack.Top();
	StackFrame& current = GetStackFrame();
	while(_stack.GetSize() > current._stackSize) {
		_stack.Pop();
	}

	if(returnValue) {
		_stack.Push(returnedValue);
	}
	else {
		_stack.Push(ScriptConstants::Null);
	}
}

void VM::Break() {
	while(true) {
		StackFrame& frame = GetStackFrame();
		
		if(frame._scriptlet->IsFunction()) {
			throw ScriptException(L"Cannot break out of function; please use a return statement!");
		}

		_scope = _scope->GetPrevious();
		bool isLoop = frame._scriptlet->IsLoop();
		_call.pop_back();
		if(isLoop) {
			break;
		}
	}
}

void VM::Execute(ref<ScriptContext> c, ref<CompiledScript> script, ref<ScriptScope> scope) {
	_script = script;
	_context = c; // weak reference
	_scope = scope;

	// maybe work with pc?
	ref<VM> vm = This<VM>();

	ref<Scriptlet> main = script->GetMainScriptlet();
	_call.push_back(GC::Hold(new StackFrame(main,0)));

	try {
		while(!_call.empty()) {
			ref<StackFrame> current = *(_call.rbegin());
			ref<Scriptlet> scriptlet = current->_scriptlet;

			if(current->_pc>=scriptlet->_code.size()) {
				_call.pop_back();
				if(!_call.empty()) {
					// attempt stack repair if necessary
					current = *(_call.rbegin());
					if(_stack.GetSize()<current->_stackSize) {
						throw ScriptException(L"Scriptlet damaged caller's stack");
					}
					else {
						//Log::Write(L"TJScript/VM", L"Pre-PR: "+_stack.Dump());
						_stack.Pop(current->_stackSize);
						//Log::Write(L"TJScript/VM", L"Post-PR: "+_stack.Dump());
					}

					if(scriptlet->IsFunction()) {
						_stack.Push(ScriptConstants::Null);
					}
				}
				
				if(scriptlet!=main) {
					_scope = _scope->GetPrevious();
				}
			}
			else {
				ref<Op> op = scriptlet->_code.at(current->_pc);
				
				if(_debug) {
					Log::Write(L"TJScript/VM/Execute",Stringify(_script->GetScriptletIndex(current->_scriptlet))+L": "+op->GetName());
					//Log::Write(L"TJScript/VM/Execute", L"    "+_stack.Dump());
				}

				current->_pc++;

				try {
					op->Execute(vm);
				}
				catch(BreakpointException&) {
					throw;
				}
			}
		}
	}
	catch(Exception& e) {
		_script = 0;
		_context = ref<ScriptContext>(0);
		_scope = 0;
		_call.clear();
		_stack.Clear();

		Log::Write(L"TJScript/VM", std::wstring(L"Stack dump: ")+_stack.Dump());
		Log::Write(L"TJScript/VM", Wcs(e.GetFile())+L"/"+Stringify(e.GetLine())+L": "+e.GetMsg());
		throw;
	}

	_script = 0;
	_context = ref<ScriptContext>(0);
	_scope = 0;
	_call.clear();
	_stack.Clear();
}