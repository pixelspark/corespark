#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

int StackFrame::Count = 0;

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

	_call.push_back(StackFrame(s, 0, (bool)p));

	if(p) {
		p->SetPrevious(_scope);
		_scope = p;
	}
}

/* Creates the current scope that new variables can be written to. If no scope was created for the 
current stack frame yet, it will create one */
ref<ScriptScope> VM::GetCurrentScopeForWriting() {
	StackFrame& frame = GetStackFrame();
	
	if(!frame._createdScope) {
		frame._createdScope = true;
		ScriptScope* ss = new ScriptScope();
		ss->SetPrevious(_scope);
		_scope = GC::Hold(ss);
	}
	return _scope;
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
		StackFrame& frame = GetStackFrame();
		bool isFunction = frame._scriptlet->IsFunction();
		
		if(frame._createdScope) {
			_scope = _scope->GetPrevious();
		}

		_call.pop_back();

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

		if(frame._createdScope) {
			_scope = _scope->GetPrevious();
		}

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
	_call.push_back(StackFrame(main, 0));
	ref<Op> op = 0;

	try {
		while(!_call.empty()) {
			StackFrame& current = GetStackFrame();
			ref<Scriptlet> scriptlet = current._scriptlet;

			if(current._pc>=scriptlet->_code.size()) {
				// Destroy the scope if this scriptlet created one
				if(scriptlet!=main && current._createdScope) {
					_scope = _scope->GetPrevious();
				}

				// Pop the stack frame
				_call.pop_back();

				// If the call stack is not empty, continu execution on the last frame
				if(!_call.empty()) {
					current = GetStackFrame();

					/* This checks if the current stack frame returns with as many items on the stack
					as when it entered this stack frame (could be +1 for return value, see return code above).
					
					TODO: not really necessary anymore, since the parser pops after each
					statement that leaves something on the stack (or it should anyway...). Maybe
					create a separate DebugExecute method with debugging and stack-checks like
					this one. If we want to remove the stackSize field from StackFrame, we should
					consider creating a separate 'stack count stack' for the debug method. */
					if(_stack.GetSize()!=current._stackSize) {
						if(_stack.GetSize()>=current._stackSize) {
							_stack.Pop(current._stackSize);
						}
						Log::Write(L"TJScript/VM", L"Pop repair");
					}

					if(scriptlet->IsFunction()) {
						_stack.Push(ScriptConstants::Null);
					}
				}
			}
			else {
				op = scriptlet->_code.at(current._pc);
				++current._pc;
				op->Execute(vm);

				if(_debug) {
					Log::Write(L"TJScript/VM/Execute",/*Stringify(_script->GetScriptletIndex(current._scriptlet))+L": "+*/op->GetName());
				}
			}
		}
	}
	catch(BreakpointException&) {
		Log::Write(L"TJScript/VM", L"A breakpoint was triggered, script execution stopped!");
		/* TODO: create something to resume the VM execution (should be possible as all state info is still intact
		at this point */
	}
	catch(Exception& e) {
		StackFrame& current = GetStackFrame();
		Log::Write(L"TJScript/VM",L"Error in scriptlet "+Stringify(_script->GetScriptletIndex(current._scriptlet))+L": "+op->GetName());
		Log::Write(L"TJScript/VM", std::wstring(L"Stack dump: ")+_stack.Dump());
		Log::Write(L"TJScript/VM", Wcs(e.GetFile())+L"/"+Stringify(e.GetLine())+L": "+e.GetMsg());

		_script = 0;
		_context = ref<ScriptContext>(0);
		_scope = 0;
		_call.clear();
		_stack.Clear();
		throw;
	}

	_script = 0;
	_context = ref<ScriptContext>(0);
	_scope = 0;
	_call.clear();
	_stack.Clear();
}