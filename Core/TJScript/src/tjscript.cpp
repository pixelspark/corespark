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

ref<Scriptable> ScriptConstants::Null;
ref<Scriptable> ScriptConstants::True;
ref<Scriptable> ScriptConstants::False;
ScriptConstantsInitializer ScriptConstants::_init;

ScriptConstantsInitializer::ScriptConstantsInitializer() {
	ScriptConstants::Null = GC::Hold(new ScriptNull());
	ScriptConstants::True = GC::Hold(new ScriptBool(true));
	ScriptConstants::False = GC::Hold(new ScriptBool(false));
}

Scriptable::~Scriptable() {
}

bool Scriptable::Set(Field field, tj::shared::ref<Scriptable> value) {
	return false;
}

ScriptletStack::~ScriptletStack() {
}