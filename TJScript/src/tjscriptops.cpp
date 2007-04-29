#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;
using namespace tj::script::ops;

/* OpPush */
OpPush::OpPush(ref<Scriptable> s) {
	_value = s;
}

std::wstring OpPush::GetName() {
	return std::wstring(L"OpPush ") + ScriptContext::GetValue<std::wstring>(_value, L"[unknown]");
}

void OpPush::Execute(ref<VM> vm) {
	vm->GetStack().Push(_value);
}

void OpPop::Execute(ref<VM> vm) {
	vm->GetStack().Pop();
}

void OpParameter::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> value = stack.Pop();
	ref<Scriptable> key = stack.Pop();
	ref<ScriptParameterList> parameter = stack.Pop();

	ref< ScriptValue<std::wstring> > keyString = key;
	parameter->Set(keyString->GetValue(), value);
	stack.Push(parameter);
}

void OpNamelessParameter::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> value = stack.Pop();
	ref<ScriptParameterList> parameter = stack.Pop();
	parameter->AddNamelessParameter(value);
	stack.Push(parameter);
}

void OpPushParameter::Execute(ref<VM> vm) {
	vm->GetStack().Push(GC::Hold(new ScriptParameterList()));
}

void OpCall::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> parameterList = stack.Pop();
	ref<ScriptParameterList> list;
	if(parameterList.IsCastableTo<ScriptParameterList>()) {
		list = parameterList;
	}
	else {
		stack.Push(parameterList);
	}

	ref< ScriptValue<std::wstring> > funcName = stack.Pop();
	ref<Scriptable> target = stack.Pop();

	ref<Scriptable> result = target->Execute(funcName->GetValue(), list);
	if(!result) {
		throw ScriptException(L"Variable does not exist on object or scope");
	}

	// only call functions when there is a parameter list given
	if(result.IsCastableTo<ScriptFunction>() && list) {
		ref<Scriptlet> scriptlet = ref<ScriptFunction>(result)->_scriptlet;
		vm->Call(scriptlet, list);
	}
	else {
		stack.Push(result);
	}
}

void OpIndex::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> index = stack.Pop();
	ref<Scriptable> object = stack.Pop();

	ref<ParameterList> pl = GC::Hold(new ParameterList());
	pl->Set(L"key", index);
	ref<Scriptable> result = object->Execute(L"get", pl);
	if(result==0) {
		throw ScriptException(L"Object does not support get(key=...) method, array index cannot be used");
	}
	stack.Push(result);
}


void OpCallGlobal::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> parameterList = stack.Pop();
	ref<ScriptParameterList> list;
	if(parameterList.IsCastableTo<ScriptParameterList>()) {
		list = parameterList;
	}
	else {
		stack.Push(parameterList);
	}

	ref< ScriptValue<std::wstring> > funcName = stack.Pop();
	ref<Scriptable> target = vm->GetCurrentScope();

	ref<Scriptable> result = target->Execute(funcName->GetValue(), list);
	if(!result) {
		throw ScriptException(L"Variable does not exist on object or scope: '"+funcName->GetValue()+L"'");
	}

	// only call functions when there is a parameter list given
	if(result.IsCastableTo<ScriptFunction>() && list) {
		ref<Scriptlet> scriptlet = ref<ScriptFunction>(result)->_scriptlet;
		vm->Call(scriptlet, list);
	}
	else {
		stack.Push(result);
	}
}

void OpSave::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> object = stack.Pop();
	ref< ScriptValue<std::wstring> > varName = stack.Pop();
	vm->GetCurrentScopeForWriting()->Set(varName->GetValue(), object);
}

void OpEquals::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> objectA = stack.Pop();
	ref<Scriptable> objectB = stack.Pop();

	bool result = false;

	if(objectA.IsCastableTo<ScriptString>() && objectB.IsCastableTo<ScriptString>()) {
		ref<ScriptString> strA = objectA;
		ref<ScriptString> strB = objectB;

		result = strA->GetValue() == strB->GetValue();
	}
	else if(objectA.IsCastableTo<ScriptInt>() && objectB.IsCastableTo<ScriptInt>()) {
		ref<ScriptInt> strA = objectA;
		ref<ScriptInt> strB = objectB;

		result = strA->GetValue() == strB->GetValue();
	}
	else if(objectA.IsCastableTo<ScriptDouble>() && objectB.IsCastableTo<ScriptDouble>()) {
		ref<ScriptDouble> strA = objectA;
		ref<ScriptDouble> strB = objectB;

		result = strA->GetValue() == strB->GetValue();
	}
	else if(objectA.IsCastableTo<ScriptBool>() && objectB.IsCastableTo<ScriptBool>()) {
		ref<ScriptBool> strA = objectA;
		ref<ScriptBool> strB = objectB;

		result = strA->GetValue() == strB->GetValue();
	}
	else if(objectA.IsCastableTo<ScriptNull>() && objectB.IsCastableTo<ScriptNull>()) {
		result = true;
	}
	else {
		result = (objectA==objectB);
	}

	stack.Push(result?ScriptConstants::True:ScriptConstants::False);
}

void OpNegate::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> top = stack.Pop();

	if(top.IsCastableTo<ScriptBool>()) {
		ref<ScriptBool> b = top;
		ref<ScriptBool> n = b->GetValue()?ScriptConstants::False:ScriptConstants::True;
		stack.Push(n);
	}
	else if(top.IsCastableTo<ScriptInt>()) {
		ref<ScriptInt> b = top;
		ref<ScriptInt> n = GC::Hold(new ScriptInt(-b->GetValue()));
		stack.Push(n);
	}
	else if(top.IsCastableTo<ScriptDouble>()) {
		ref<ScriptDouble> b = top;
		ref<ScriptDouble> n = GC::Hold(new ScriptDouble(-b->GetValue()));
		stack.Push(n);
	}
	else {
		stack.Push(ScriptConstants::False);
	}
}

void OpAdd::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> a = stack.Pop();
	ref<Scriptable> b = stack.Pop();

	// if any of the parameters is a string, do concatenation
	if(a.IsCastableTo<ScriptString>() || b.IsCastableTo<ScriptString>()) {
		std::wstring as = ScriptContext::GetValue<std::wstring>(a,L"");
		std::wstring bs = ScriptContext::GetValue<std::wstring>(b,L"");
		stack.Push(GC::Hold(new ScriptString(bs+as)));
	}
	else {
		double va = 0.0;
		double vb = 0.0;

		if(a.IsCastableTo<ScriptDouble>()) {
			va = ref<ScriptDouble>(a)->GetValue();
		}
		else if(a.IsCastableTo<ScriptInt>()) {
			va = (double)ref<ScriptInt>(a)->GetValue();
		}
		else {
			va = ScriptContext::GetValue<double>(a,0.0);
		}
		
		if(b.IsCastableTo<ScriptDouble>()) {
			vb = ref<ScriptDouble>(b)->GetValue();
		}
		else if(b.IsCastableTo<ScriptInt>()) {
			vb = (double)ref<ScriptInt>(b)->GetValue();
		}
		else {
			vb = ScriptContext::GetValue<double>(b,0.0);
		}

		stack.Push(GC::Hold(new ScriptDouble(va+vb)));
	}
}

void OpSub::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> a = stack.Pop();
	ref<Scriptable> b = stack.Pop();

	double va = 0.0;
	double vb = 0.0;

	if(a.IsCastableTo<ScriptDouble>()) {
		va = ref<ScriptDouble>(a)->GetValue();
	}
	else if(a.IsCastableTo<ScriptInt>()) {
		va = (double)ref<ScriptInt>(a)->GetValue();
	}
	else {
		va = ScriptContext::GetValue<double>(a,0.0);
	}
	
	if(b.IsCastableTo<ScriptDouble>()) {
		vb = ref<ScriptDouble>(b)->GetValue();
	}
	else if(b.IsCastableTo<ScriptInt>()) {
		vb = (double)ref<ScriptInt>(b)->GetValue();
	}
	else {
		vb = ScriptContext::GetValue<double>(b,0.0);
	}

	stack.Push(GC::Hold(new ScriptDouble(vb-va)));
}

void OpDiv::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> a = stack.Pop();
	ref<Scriptable> b = stack.Pop();

	double va = 0.0;
	double vb = 0.0;

	if(a.IsCastableTo<ScriptDouble>()) {
		va = ref<ScriptDouble>(a)->GetValue();
	}
	else if(a.IsCastableTo<ScriptInt>()) {
		va = (double)ref<ScriptInt>(a)->GetValue();
	}
	else {
		va = ScriptContext::GetValue<double>(a,0.0);
	}
	
	if(b.IsCastableTo<ScriptDouble>()) {
		vb = ref<ScriptDouble>(b)->GetValue();
	}
	else if(b.IsCastableTo<ScriptInt>()) {
		vb = (double)ref<ScriptInt>(b)->GetValue();
	}
	else {
		vb = ScriptContext::GetValue<double>(b,0.0);
	}

	if(va==0.0) {
		throw ScriptException(L"Division by zero");
	}

	stack.Push(GC::Hold(new ScriptDouble(vb/va)));
}

void OpMul::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> a = stack.Pop();
	ref<Scriptable> b = stack.Pop();

	double va = 0.0;
	double vb = 0.0;

	if(a.IsCastableTo<ScriptDouble>()) {
		va = ref<ScriptDouble>(a)->GetValue();
	}
	else if(a.IsCastableTo<ScriptInt>()) {
		va = (double)ref<ScriptInt>(a)->GetValue();
	}
	else {
		va = ScriptContext::GetValue<double>(a,0.0);
	}
	
	if(b.IsCastableTo<ScriptDouble>()) {
		vb = ref<ScriptDouble>(b)->GetValue();
	}
	else if(b.IsCastableTo<ScriptInt>()) {
		vb = (double)ref<ScriptInt>(b)->GetValue();
	}
	else {
		vb = ScriptContext::GetValue<double>(b,0.0);
	}

	stack.Push(GC::Hold(new ScriptDouble(va*vb)));
}

OpBranchIf::OpBranchIf(int scriptlet) {
	_scriptlet = scriptlet;
}

void OpBranchIf::Execute(tj::shared::ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> top = stack.Top();
	
	bool r = ScriptContext::GetValue<bool>(top,false);
	if(r) {
		vm->Call(_scriptlet);
	}
}

// OpLoadScriptlet
OpLoadScriptlet::OpLoadScriptlet(int scriptlet) {
	_scriptlet = scriptlet;
}

void OpLoadScriptlet::Execute(tj::shared::ref<VM> vm) {
	ref<Scriptlet> sc = vm->GetScript()->GetScriptlet(_scriptlet);
	vm->GetStack().Push(GC::Hold(new ScriptFunction(sc)));
}

void OpReturn::Execute(tj::shared::ref<VM> vm) {
	vm->Return(false);
}

void OpReturnValue::Execute(ref<VM> vm) {
	vm->Return(true);
}

// OpGreaterThan
void OpGreaterThan::Execute(ref<VM> vm) {
	ref<Scriptable> a = vm->GetStack().Pop();
	ref<Scriptable> b = vm->GetStack().Pop();

	if(a.IsCastableTo<ScriptDouble>() && b.IsCastableTo<ScriptDouble>()) {
		double va = ref<ScriptDouble>(a)->GetValue();
		double vb = ref<ScriptDouble>(b)->GetValue();
		bool result = vb>va;
		vm->GetStack().Push(result?ScriptConstants::True:ScriptConstants::False);
	}
	else {
		vm->GetStack().Push(ScriptConstants::Null);
	}
}

// OpLessThan
void OpLessThan::Execute(ref<VM> vm) {
	ref<Scriptable> a = vm->GetStack().Pop();
	ref<Scriptable> b = vm->GetStack().Pop();

	if(a.IsCastableTo<ScriptDouble>() && b.IsCastableTo<ScriptDouble>()) {
		double va = ref<ScriptDouble>(a)->GetValue();
		double vb = ref<ScriptDouble>(b)->GetValue();
		bool result = vb<va;
		vm->GetStack().Push(result?ScriptConstants::True:ScriptConstants::False);
	}
	else {
		vm->GetStack().Push(ScriptConstants::Null);
	}
}

// OpOr
void OpOr::Execute(ref<VM> vm) {
	ref<Scriptable> a = vm->GetStack().Pop();
	ref<Scriptable> b = vm->GetStack().Pop();

	bool ba = ScriptContext::GetValue<bool>(a, false);
	bool bb = ScriptContext::GetValue<bool>(b, false);

	vm->GetStack().Push(GC::Hold(new ScriptBool(ba||bb)));
}

// OpAnd
void OpAnd::Execute(ref<VM> vm) {
	ref<Scriptable> a = vm->GetStack().Pop();
	ref<Scriptable> b = vm->GetStack().Pop();

	bool ba = ScriptContext::GetValue<bool>(a, false);
	bool bb = ScriptContext::GetValue<bool>(b, false);

	vm->GetStack().Push((ba&&bb)?ScriptConstants::True:ScriptConstants::False);
}

// OpXor: TT=>F FF=>F TF=>T FT=>T
// Xor: (a||b) && !(a==b)
void OpXor::Execute(ref<VM> vm) {
	ref<Scriptable> a = vm->GetStack().Pop();
	ref<Scriptable> b = vm->GetStack().Pop();

	bool ba = ScriptContext::GetValue<bool>(a, false);
	bool bb = ScriptContext::GetValue<bool>(b, false);
	bool result = ((ba||bb) && !(ba==bb));
	vm->GetStack().Push(result?ScriptConstants::True:ScriptConstants::False);
}

void OpBreak::Execute(ref<VM> vm) {
	vm->Break();
}

void OpNew::Execute(ref<VM> vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> parameterList = stack.Pop();
	ref<ScriptParameterList> list;
	if(parameterList.IsCastableTo<ScriptParameterList>()) {
		list = parameterList;
	}
	else {
		stack.Push(parameterList);
	}

	ref< ScriptString > funcName = stack.Pop();
	
	ref<Scriptable> instance = vm->GetContext()->GetType(funcName->GetValue())->Construct(list);
	stack.Push(instance);
}

void OpIterate::Execute(ref<VM> vm) {
	ref<Scriptable> iterable = vm->GetStack().Pop();
	ref<Scriptable> varName = vm->GetStack().Pop();
	vm->GetStack().Push(varName);
	vm->GetStack().Push(iterable);
	
	ref<Scriptable> value = iterable->Execute(L"next", 0);
	if(value==0) {
		throw ScriptException(L"Object does not support iteration");
	}
	else if(!value.IsCastableTo<ScriptNull>()) {
		// set pc of this stack frame one back, so this instruction will be called again
		StackFrame& frame = vm->GetStackFrame();
		frame._pc--;

		//set variable
		vm->GetCurrentScopeForWriting()->Set(ref<ScriptString>(varName)->GetValue(), value);

		vm->Call(_scriptlet);
	}
	else {
		vm->GetStack().Pop();
		vm->GetStack().Pop();
	}
}