#include "../include/internal/tjscript.h"
#include "../include/types/tjstatictypes.h"
using namespace tj::shared;
using namespace tj::script;

namespace tj {
	namespace script {
		class ScriptMapIterator: public Scriptable {
			public:
				ScriptMapIterator(ref<ScriptMap> x, bool iterateKey) {
					_array = x;
					_it = _array->_array.begin();
					_key = iterateKey;
				}

				virtual ~ScriptMapIterator() {
				}

				virtual ref<Scriptable> Execute(Command c, ref<ParameterList> p) {
					ThreadLock lock(&(_array->_lock));
					if(c==L"toString") {
						return GC::Hold(new ScriptString(L"[ScriptMapIterator]"));
					}
					else if(c==L"next") {
						if(_it!=_array->_array.end()) {
							if(_key) {
								std::wstring key = (*_it).first;
								++_it;
								return GC::Hold(new ScriptString(key));
							}
							else {
								ref<Scriptable> val = (*_it).second;
								++_it;
								return val;
							}
						}
						return ScriptConstants::Null; // no more elements
					}
					return 0;
				};

				ref<ScriptMap> _array;
				std::map< std::wstring, ref<Scriptable> >::iterator _it;
				bool _key;
		};
	}
}

ScriptMapType::~ScriptMapType() {
}

ref<Scriptable> ScriptMapType::Construct(ref<ParameterList> p) {
	ref<ScriptMap> ar = GC::Hold(new ScriptMap());
	return ar;
}

ScriptMap::ScriptMap() {
}

ScriptMap::~ScriptMap() {
}

std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator ScriptMap::GetBegin() {
	return _array.begin();
}

std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator ScriptMap::GetEnd() {
	return _array.end();
}

void ScriptMap::Initialize() {
	Bind(L"size", &ScriptMap::Size);
	Bind(L"get", &ScriptMap::Get);
	Bind(L"toString", &ScriptMap::ToString);
	Bind(L"keys", &ScriptMap::Keys);
	Bind(L"values", &ScriptMap::Values);
}

ref<Scriptable> ScriptMap::Size(ref<ParameterList> p) {
	ThreadLock lock(&_lock);
	return GC::Hold(new ScriptInt((int)_array.size()));
}

bool ScriptMap::Set(Field field, ref<Scriptable> value) {
	_array[field] = value;
	return true;
}

ref<Scriptable> ScriptMap::Get(ref<ParameterList> p) {
	ThreadLock lock(&_lock);
	static const Parameter<std::wstring> PKey(L"key", 0);

	std::wstring key = PKey.Require(p,L"");
	if(_array.find(key)!=_array.end()) {
		return _array[key];
	}
	
	return ScriptConstants::Null;
}

ref<Scriptable> ScriptMap::ToString(ref<ParameterList> p) {
	ThreadLock lock(&_lock);

	std::wostringstream wos;
	wos << L"[";
	std::map<std::wstring, ref<Scriptable> >::iterator it = _array.begin();
	while(it!=_array.end()) {
		wos << it->first << L"=" << ScriptContext::GetValue<std::wstring>(it->second, L"?") << L" ";
		++it;
	}
	wos << L"]";

	return GC::Hold(new ScriptString(wos.str()));
}

ref<Scriptable> ScriptMap::Keys(ref<ParameterList> p) {
	ThreadLock lock(&_lock);
	return GC::Hold(new ScriptMapIterator(this, true));
}

ref<Scriptable> ScriptMap::Values(ref<ParameterList> p) {
	ThreadLock lock(&_lock);
	return GC::Hold(new ScriptMapIterator(this, false));
}