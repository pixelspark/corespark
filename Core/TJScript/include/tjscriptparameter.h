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
 
 #ifndef _TJSCRIPTPARAMETER_H
#define _TJSCRIPTPARAMETER_H

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ParameterException: public ScriptException {
			public:
				ParameterException(std::wstring parameter);
				virtual ~ParameterException();
		};

		template<typename T> class Parameter {
			public:
				inline Parameter(Field name, int namelessIndex = -1) {	
					if(namelessIndex>=0) {
						_hasNamelessIndex = true;
						_namelessIndex = tj::shared::Stringify(namelessIndex);
					}
					
					_name = name;
				}

				inline T Require(tj::shared::ref<ParameterList> p, T defaultValue) const {
					if(!p) {
						throw ParameterException(_name);
					}
				
					tj::shared::ref<Scriptable> v = p->Get(_name);
					if(!v) {
						if(_hasNamelessIndex) {
							v = p->Get(_namelessIndex);

							if(!v) {
								throw ParameterException(L"");
							}
						}
						else {
							throw ParameterException(L"");
						}
					}
					
					return ScriptContext::GetValue<T>(v, defaultValue);
				}

				T Get(tj::shared::ref<ParameterList> p, T defaultValue) const {
					if(p) {
						tj::shared::ref<Scriptable> v = p->Get(_name);
						
						if(!v && _hasNamelessIndex) {
							v = p->Get(_namelessIndex);
						}
						
						if(!v) {
							return defaultValue;
						}

						return ScriptContext::GetValue<T>(v, defaultValue);
					}
					return defaultValue;
				}

				inline bool Exists(tj::shared::ref<ParameterList> p) const {
					if(!p) return false;

					return p->Exists(_name) || (_hasNamelessIndex && p->Exists(_namelessIndex));
				}

				inline tj::shared::ref<Scriptable> Get(tj::shared::ref<ParameterList> p) const {
					if(!p) return 0;

					ref<Scriptable> val = p->Get(_name);
					if(!val && _hasNamelessIndex) {
						val = p->Get(_namelessIndex);
					}
					return val;
				}

			protected:
				bool _hasNamelessIndex;
				std::wstring _namelessIndex;
				std::wstring _name;
		};
	}
}

#endif