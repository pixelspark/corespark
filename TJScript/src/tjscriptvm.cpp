#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

VM::VM(int stackLimit) {
	_stack = GC::Hold(new ScriptStack(stackLimit));
	_global = GC::Hold(new ScriptScope());
	_debug = false;
}

VM::~VM() {
}

ref<ScriptContext> VM::GetContext() {
	ref<ScriptContext> c = _context.Reference();
	if(!c) {
		throw ScriptException(L"Context not set in VM");
	}

	return c;
}

void VM::SetDebug(bool d) {
	_debug = d;
}

void VM::Call(ref<Scriptlet> s, ref<ScriptParameterList> p) {
	// set stack size in caller for stack recovery
	if(!_call.empty()) {
		ref<StackFrame> current = *(_call.rbegin());
		current->_stackSize = _stack->GetSize();
	}
	_call.push_back(GC::Hold(new StackFrame(s,0)));

	ScriptScope* ss = new ScriptScope(p);
	ss->SetPrevious(_global);
	_global = GC::Hold(ss);
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
		ref<StackFrame> frame = *(_call.rbegin());
		_call.pop_back();
		_global = _global->GetPrevious();
		if(frame->_scriptlet->IsFunction()) {
			break;
		}
	}

	ref<Scriptable> returnedValue = _stack->Top();
	ref<StackFrame> current = *(_call.rbegin());
	while(_stack->GetSize() > current->_stackSize) {
		_stack->Pop();
	}

	if(returnValue) {
		_stack->Push(returnedValue);
	}
	else {
		_stack->Push(ScriptConstants::Null());
	}
}

void VM::Break() {
	while(true) {
		ref<StackFrame> frame = *(_call.rbegin());
		if(frame->_scriptlet->IsFunction()) {
			throw ScriptException(L"Cannot break out of function; please use a return statement!");
		}
		_call.pop_back();
		_global = _global->GetPrevious();
		if(frame->_scriptlet->IsLoop()) {
			break;
		}
	}
}

ref<CompiledScript> VM::GetScript() {
	return _script;
}

void VM::Execute(ref<ScriptContext> c, ref<CompiledScript> script) {
	_script = script;
	_context = c; // weak reference

	// maybe work with pc?
	ref<VM> vm = This<VM>();
	if(!_call.empty()) {
		_call.clear();
	}

	if(!_stack->IsEmpty()) {
		_stack->Clear();
	}

	ref<Scriptlet> main = script->GetMainScriptlet();
	_call.push_back(GC::Hold(new StackFrame(main,0)));

	try {
		while(!_call.empty()) {
			ref<StackFrame> current = *(_call.rbegin());
			ref<Scriptlet> scriptlet = current->_scriptlet;

			if(current->_pc>=scriptlet->_code.size()) {
				if(_debug) {
					Log::Write(L"TJScript/VM", L"Scriptlet ended");
				}
				_call.pop_back();
				if(!_call.empty()) {
					// attempt stack repair if necessary
					current = *(_call.rbegin());
					if(_stack->GetSize()<current->_stackSize) {
						throw ScriptException(L"Scriptlet damaged caller's stack");
					}
					else if(_stack->GetSize()>current->_stackSize) {
						while(_stack->GetSize()>current->_stackSize) {
							_stack->Pop();
							if(_debug) {
								Log::Write(L"TJScript/VM", L"Pop repair");
							}
						}
					}

					if(scriptlet->IsFunction()) {
						_stack->Push(ScriptConstants::Null());
					}
				}
				
				if(scriptlet!=main) {
					_global = _global->GetPrevious();
				}
			}
			else {
				ref<Op> op = scriptlet->_code.at(current->_pc);
				
				if(_debug) {
					Log::Write(L"TJScript/VM/Execute",Stringify(_script->GetScriptletIndex(current->_scriptlet))+L": "+op->GetName());
				}

				current->_pc++;

				try {
					op->Execute(vm);
				}
				catch(BreakpointException&) {
					//std::wostringstream wos;
					//wos << L"A breakpoint was triggered. Stack dump: " << _stack->Dump();
					//...
				}
			}
		}
	}
	catch(Exception&) {
		_script = 0;
		Log::Write(L"TJScript/VM", std::wstring(L"Stack dump: ")+_stack->Dump());
		throw;
	}

	_script = 0;
	_context = ref<ScriptContext>(0);
}

ref<ScriptStack> VM::GetStack() {
	return _stack;
}

void VM::SetGlobal(ref<Scriptable> g) {
	_global->SetPrevious(g);
}

ref<ScriptScope> VM::GetGlobal() {
	return _global;
}

ref<StackFrame> VM::GetStackFrame() {
	return *(_call.rbegin());
}

/* Scriptlet */
Scriptlet::Scriptlet(ScriptletType t) {
	_type = t;
}

Scriptlet::~Scriptlet() {
}

void Scriptlet::Add(ref<Op> op) {
	_code.push_back(op);
}