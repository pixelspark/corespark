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
	_stack[_sp] = 0;
	_sp--;
	return last;
}

void ScriptStack::Pop(int size) {
	assert(size>=0);

	if(size > _sp+1) {
		throw ScriptException(L"Stack underflow");
	}

	_stack[_sp] = 0;
	_sp = size-1;
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