#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

CompiledScript::CompiledScript() {
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
		it++;
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