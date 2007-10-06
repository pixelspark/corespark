#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

CompiledScript::CompiledScript(ScriptContext* cc) {
	assert(cc!=0);
	_creatingContext = cc;
}

CompiledScript::~CompiledScript() {
}

ref<Scriptlet> CompiledScript::CreateScriptlet(ScriptletType type) {
	ref<Scriptlet> s = GC::Hold(new Scriptlet(type));
	_scriptlets.push_back(s);
	return s;
}

ref<Scriptlet> CompiledScript::GetScriptlet(int i) {
	return _scriptlets.at(i);
}

int CompiledScript::GetScriptletIndex(ref<Scriptlet> scriptlet) {
	std::vector< ref<Scriptlet> >::iterator it = _scriptlets.begin();
	int idx = 0;
	while(it!=_scriptlets.end()) {
		ref<Scriptlet> s = *it;
		if(s==scriptlet) {
			return idx;
		}
		++it;
		idx++;
	}

	throw ScriptException(L"Scriptlet not found in compiled script");
}

int CompiledScript::GetScriptletCount() const {
	return (int)_scriptlets.size();
}

void CompiledScript::Optimize() {
	/* possible optimizations that can be done at compile-time (which means here):
	- Removal of empty scriptlets, and the calls referring to them (OpBranchIf should probably be
	  replaced with a few OpPops)

	- Removal of OpPush, OpPushGlobal at the end of a scriptlet after the last OpCall/OpBranchIf/OpSave/...

	- (Advanced) evaluation of static expressions

	- (Advanced) evaluation and elimination of if/else expressions which are static
	*/
}

ref<Scriptlet> CompiledScript::GetMainScriptlet() {
	return _scriptlets.at(0);
}

template<> Scriptlet& Scriptlet::Add(const std::wstring& x) {
	Add<unsigned int>((unsigned int)x.length());
	std::wstring::const_iterator it = x.begin();
	while(it!=x.end()) {
		wchar_t c = *it;
		Add<wchar_t>(c);
		++it;
	}
	Add<wchar_t>((wchar_t)0);
	return *this;
}

// TODO: overflow check
template<> wchar_t* Scriptlet::Get(unsigned int& position) const {
	unsigned int length = Get<unsigned int>(position);
	if(position+length >= _size) {
		Throw("Scriptlet::Get overflow (tried to get a string with length longer than the script)", ExceptionTypeError);
	}

	wchar_t* str = reinterpret_cast<wchar_t*>(&(_code[position]));
	position += (length+1)*sizeof(wchar_t);
	return str;
}