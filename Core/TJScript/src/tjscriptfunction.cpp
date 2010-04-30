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

ScriptFunction::ScriptFunction(ref<Scriptlet> s) {
	_scriptlet = s;
}

ref<Scriptable> ScriptFunction::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[function]"));
}

ScriptFunction::~ScriptFunction() {
}

void ScriptFunction::Initialize() {
	Bind(L"toString", &ScriptFunction::SToString);
}

/** ScriptDelegate **/
ScriptDelegate::ScriptDelegate(ref<CompiledScript> sc, ref<ScriptContext> ctx) {
	assert(sc);
	_cs = sc;
	_context = ctx;
}

ScriptDelegate::~ScriptDelegate() {
}

ref<CompiledScript> ScriptDelegate::GetScript() {
	return _cs;
}

ref<ScriptContext> ScriptDelegate::GetContext() {
	return _context;
}

ref<Scriptable> ScriptDelegate::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[delegate]"));
}

void ScriptDelegate::Initialize() {
	Bind(L"toString", &ScriptDelegate::SToString);
}

/** ScriptFuture **/
ScriptFuture::ScriptFuture(ref<CompiledScript> cs, ref<ScriptContext> originalContext): 
	_cs(cs),
	_originalContext(originalContext),
	_isConcrete(false),
	_scope(GC::Hold(new ScriptScope())) {
}

ScriptFuture::~ScriptFuture() {
}

bool ScriptFuture::IsConcrete() {
	ThreadLock lock(&_lock);
	return _isConcrete;
}

ref<Scriptable> ScriptFuture::GetReturnValue() {
	ThreadLock lock(&_lock);
	if(!IsConcrete()) {
		Throw(L"Cannot get return value of non-concrete future", ExceptionTypeError);
	}
	return _returnValue;
}

void ScriptFuture::AddDependency(Field field, strong<ScriptFuture> fut) {
	ThreadLock lock(&_lock);
	if(IsConcrete()) {
		Throw(L"Cannot add a dependency to an already complete future", ExceptionTypeError);
	}
	DependsOn(ref<Future>(fut));
	_scope->Set(field, fut);
}

void ScriptFuture::AddVariable(Field field, strong<Scriptable> s) {
	ThreadLock lock(&_lock);
	if(IsConcrete()) {
		Throw(L"Cannot add a variable to an already complete future", ExceptionTypeError);
	}
	_scope->Set(field, s);
}

void ScriptFuture::Run() {
	ThreadLock lock(&_lock);
	if(!IsConcrete()) {
		// TODO: some kind of context stacking is necessary, because the original context now contains
		// type definitions (_types) that the new context does not have.
		ref<ScriptContext> ctx = GC::Hold(new ScriptContext(_originalContext->GetGlobal()));
		ctx->SetDispatcher(ctx->GetDispatcher());

		// ctx->Execute can throw exceptions; they are caught in Task::Run and set the task's state to TaskFailed.
		// The value() script function (in ScriptScope::Execute) will throw an exception again if a task failed.
		_returnValue = ctx->Execute(_cs, _scope);

		_isConcrete = true;
		_finished.Signal();
	}
}

bool ScriptFuture::WaitForCompletion() {
	if(IsConcrete()) {
		return true;
	}

	bool c = _finished.Wait();
	if(c) {
		return IsConcrete();
	}
	return false;
}

void ScriptFuture::Initialize() {
	Bind(L"toString", &ScriptFuture::SToString);
}

ref<Scriptable> ScriptFuture::SToString(ref<ParameterList> p) {
	if(IsConcrete()) {
		return GC::Hold(new ScriptString(L"[concrete future]"));
	}
	return GC::Hold(new ScriptString(L"[future]"));
}