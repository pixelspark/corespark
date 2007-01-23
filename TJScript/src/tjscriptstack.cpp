#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ScriptStack::~ScriptStack() {
	delete[] _stack;
}

ref<Scriptable> ScriptStack::Pop() {
	if(_sp<0) {
		throw ScriptException(L"Stack underflow");
	}
	
	ref<Scriptable> last = _stack[_sp];
	_stack[_sp] = 0; // so the reference will be destroyed
	_sp--;
	return last;
}

void ScriptStack::Pop(int size) {
	assert(size>=0);

	if(_sp<0) {
		throw ScriptException(L"Stack underflow");
	}

	for(;_sp>size;_sp--) {
		_stack[_sp] = 0;
	}
}

void ScriptStack::Clear() {
	while(_sp>-1) {
		_stack[_sp] = 0;
		_sp--;
	}
}

std::wstring ScriptStack::Dump() {
	std::wostringstream wos;
	
	int msp = _sp;
	for(;msp>0;msp--) {
		ref<Scriptable> s = _stack[msp];
		if(!s) {
			wos << L"[Nothing]; ";
		}
		else {
			wos << ScriptContext::GetValue<std::wstring>(s, L"[unknown]") << "; ";
		}
	}

	return wos.str();
}