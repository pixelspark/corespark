#include "../include/internal/tjscript.h"
#include <limits>
using namespace tj::script;
using namespace tj::shared;

void OpNopHandler(VM* vm) {
}

void OpPushStringHandler(VM* vm) {
	StackFrame* sf = vm->GetStackFrame();
	//std::wstring value(sf->_scriptlet->Get<wchar_t*>(sf->_pc));
	LiteralIdentifier id = sf->_scriptlet->Get<LiteralIdentifier>(sf->_pc);
	vm->GetStack().Push(sf->_scriptlet->GetLiteral(id));
}

void OpPushDoubleHandler(VM* vm) {
	StackFrame* sf = vm->GetStackFrame();
	LiteralIdentifier id = sf->_scriptlet->Get<LiteralIdentifier>(sf->_pc);
	vm->GetStack().Push(sf->_scriptlet->GetLiteral(id));
}

void OpPushDelegateHandler(VM* vm) {
	StackFrame* sf = vm->GetStackFrame();
	LiteralIdentifier id = sf->_scriptlet->Get<LiteralIdentifier>(sf->_pc);
	vm->GetStack().Push(sf->_scriptlet->GetLiteral(id));
}

void OpPushTrueHandler(VM* vm) {
	vm->GetStack().Push(ScriptConstants::True);
}

void OpPushFalseHandler(VM* vm) {
	vm->GetStack().Push(ScriptConstants::False);
}

void OpPushIntHandler(VM* vm) {
	StackFrame* sf = vm->GetStackFrame();
	int value = sf->_scriptlet->Get<int>(sf->_pc);
	vm->GetStack().Push(GC::Hold(new ScriptInt(value)));
}

void OpPushNullHandler(VM* vm) {
	vm->GetStack().Push(ScriptConstants::Null);
}

void OpPopHandler(VM* vm) {
	vm->GetStack().Pop();
}

void OpCallHandler(VM* vm) {
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

void OpCallGlobalHandler(VM* vm) {
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

void OpNewHandler(VM* vm) {
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

void OpSaveHandler(VM* vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> object = stack.Pop();
	ref< ScriptValue<std::wstring> > varName = stack.Pop();
	vm->GetCurrentScopeForWriting()->Set(varName->GetValue(), object);
}

void OpEqualsHandler(VM* vm) {
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

void OpNegateHandler(VM* vm) {
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

void OpAddHandler(VM* vm) {
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

void OpSubHandler(VM* vm) {
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

void OpMulHandler(VM* vm) {
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

void OpDivHandler(VM* vm) {
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
		stack.Push(GC::Hold(new ScriptDouble(std::numeric_limits<double>::quiet_NaN())));
	}
	else {
		stack.Push(GC::Hold(new ScriptDouble(vb/va)));
	}
}

void OpAndHandler(VM* vm) {
	ref<Scriptable> a = vm->GetStack().Pop();
	ref<Scriptable> b = vm->GetStack().Pop();

	bool ba = ScriptContext::GetValue<bool>(a, false);
	bool bb = ScriptContext::GetValue<bool>(b, false);

	vm->GetStack().Push((ba&&bb)?ScriptConstants::True:ScriptConstants::False);
}

void OpOrHandler(VM* vm) {
	ref<Scriptable> a = vm->GetStack().Pop();
	ref<Scriptable> b = vm->GetStack().Pop();

	bool ba = ScriptContext::GetValue<bool>(a, false);
	bool bb = ScriptContext::GetValue<bool>(b, false);

	vm->GetStack().Push(GC::Hold(new ScriptBool(ba||bb)));
}

void OpBranchIfHandler(VM* vm) {
	ScriptStack& stack = vm->GetStack();
	StackFrame* frame = vm->GetStackFrame();
	int  scriptlet = frame->_scriptlet->Get<int>(frame->_pc);
	ref<Scriptable> top = stack.Top();
	
	bool r = ScriptContext::GetValue<bool>(top,false);
	if(r) {
		vm->Call(scriptlet);
	}
}

void OpParameterHandler(VM* vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> value = stack.Pop();
	ref<Scriptable> key = stack.Pop();
	ref<ScriptParameterList> parameter = stack.Pop();

	ref< ScriptValue<std::wstring> > keyString = key;

	parameter->Set(keyString->GetValue(), value);
	stack.Push(parameter);
}

void OpNamelessParameterHandler(VM* vm) {
	ScriptStack& stack = vm->GetStack();
	ref<Scriptable> value = stack.Pop();
	ref<ScriptParameterList> parameter = stack.Pop();
	parameter->AddNamelessParameter(value);
	stack.Push(parameter);
}

void OpLoadScriptletHandler(VM* vm) {
	StackFrame* frame = vm->GetStackFrame();
	int id = frame->_scriptlet->Get<int>(frame->_pc);

	ref<Scriptlet> sc = vm->GetScript()->GetScriptlet(id);
	vm->GetStack().Push(GC::Hold(new ScriptFunction(sc)));
}

void OpReturnHandler(VM* vm) {
	vm->Return(false);
}

void OpReturnValueHandler(VM* vm) {
	vm->Return(true);
}

void OpGreaterThanHandler(VM* vm) {
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

void OpLessThanHandler(VM* vm) {
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

void OpXorHandler(VM* vm) {
	ref<Scriptable> a = vm->GetStack().Pop();
	ref<Scriptable> b = vm->GetStack().Pop();

	bool ba = ScriptContext::GetValue<bool>(a, false);
	bool bb = ScriptContext::GetValue<bool>(b, false);
	bool result = ((ba||bb) && !(ba==bb));
	vm->GetStack().Push(result?ScriptConstants::True:ScriptConstants::False);
}

void OpBreakHandler(VM* vm) {
	vm->Break();
}

void OpIndexHandler(VM* vm) {
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

void OpIterateHandler(VM* vm) {
	ref<Scriptable> iterable = vm->GetStack().Pop();
	ref<Scriptable> varName = vm->GetStack().Pop();
	vm->GetStack().Push(varName);
	vm->GetStack().Push(iterable);

	StackFrame* frame = vm->GetStackFrame();
	int mypc = frame->_pc; // previous instruction
	int scriptlet = frame->_scriptlet->Get<int>(frame->_pc);
	
	ref<Scriptable> value = iterable->Execute(L"next", 0);
	if(value==0) {
		throw ScriptException(L"Object does not support iteration");
	}
	else if(!value.IsCastableTo<ScriptNull>()) {
		// set pc of this stack frame one back, so this instruction will be called again
		StackFrame* frame = vm->GetStackFrame();
		frame->_pc = mypc-(sizeof(int)/sizeof(char)); // move back one instruction, so that this instruction will be called again when the scriptlet returns

		//set variable
		vm->GetCurrentScopeForWriting()->Set(ref<ScriptString>(varName)->GetValue(), value);

		vm->Call(scriptlet);
	}
	else {
		vm->GetStack().Pop();
		vm->GetStack().Pop();
	}
}

void OpPushParameterHandler(VM* vm) {
	vm->GetStack().Push(GC::Hold(new ScriptParameterList()));
}

void OpSetFieldHandler(VM* vm) {
	ScriptStack& st = vm->GetStack();
	ref<Scriptable> value = st.Pop();
	ref<Scriptable> ident = st.Pop();
	ref<Scriptable> target = st.Pop();

	ref<ScriptString> identString = ident;
	if(!identString) {
		throw ScriptException(L"Identifier is not a string; cannot set field!");
	}

	if(!target->Set(identString->GetValue(), value)) {
		throw ScriptException(L"Object is not mutable; cannot set field "+identString->GetValue()+L"!");
	}

	st.Push(value);
}

const wchar_t* Ops::Names[Ops::_OpLast] = {L"OpNop", L"OpPushString", L"OpPushDouble", L"OpPushTrue", L"OpPushFalse", L"OpPushInt", L"OpPushNull", L"OpPop", 
L"OpCall", L"OpCallGlobal", L"OpNew", L"OpSave", L"OpEquals", L"OpNegate", L"OpAdd", L"OpSub", 
L"OpMul", L"OpDiv", L"OpAnd",L"OpOr", L"OpBranchIf", L"OpParameter", L"OpNamelessParameter", L"OpPushParameter",
L"OpLoadScriptlet", L"OpReturn", L"OpReturnValue", L"OpGreaterThan", L"OpLessThan", L"OpXor",
L"OpBreak", L"OpIndex", L"OpIterate", L"OpPushDelegate", L"OpSetField" };

Ops::OpHandler Ops::Handlers[Ops::_OpLast] = {OpNopHandler,OpPushStringHandler,OpPushDoubleHandler,
OpPushTrueHandler, OpPushFalseHandler,OpPushIntHandler, OpPushNullHandler, OpPopHandler,OpCallHandler,OpCallGlobalHandler,OpNewHandler,
OpSaveHandler,OpEqualsHandler,OpNegateHandler,OpAddHandler,OpSubHandler,
OpMulHandler,OpDivHandler,OpAndHandler,OpOrHandler,OpBranchIfHandler,
OpParameterHandler,OpNamelessParameterHandler,OpPushParameterHandler, OpLoadScriptletHandler, OpReturnHandler,
OpReturnValueHandler,OpGreaterThanHandler,OpLessThanHandler,OpXorHandler,OpBreakHandler
,OpIndexHandler,OpIterateHandler, OpPushDelegateHandler, OpSetFieldHandler};

