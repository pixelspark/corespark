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
 
 #include "../include/tjdbpersistence.h"
using namespace tj::shared;
using namespace tj::db;

/** EntityContext **/
std::map< EntityType, const Schema* >* EntityContext::_schemas = 0;

EntityContext::EntityContext() {
}

EntityContext::~EntityContext() {
}

void EntityContext::RegisterSchema(const EntityType& et, const Schema* s) {
	if(_schemas==0) {
		_schemas = new std::map< EntityType, const Schema*>();
	}
	if(et!=L"") {
		(*_schemas)[et] = s;
	}
}

void EntityContext::SetCacheForEntityType(const EntityType& ot, strong<EntityCache> ec) {
	ThreadLock lock(&_cacheLock);
	_cache[ot] = ec;
}

ref<EntityCache> EntityContext::GetCacheForEntityType(const EntityType& ot) {
	ThreadLock lock(&_cacheLock);
	std::map<EntityType, ref<EntityCache> >::iterator it = _cache.find(ot);
	if(it!=_cache.end()) {
		return it->second;
	}
	return null;
}

/** Schema **/
Schema::Schema(): _super(0) {
}

Schema::~Schema() {
}

void Schema::SetSuperSchema(const Schema* ss) {
	_super = ss;
}

bool Schema::IsSubSchema() const {
	return _super!=0;
}

const Schema* Schema::GetSuperSchema() const {
	return _super;
}

const EntityType& Schema::GetEntityType() const {
	return _type;
}

/** Entity **/
Entity::~Entity() {
}

void Entity::OnMaterialized() {
}

/** EntityCache **/
EntityCache::~EntityCache() {
}

void EntityCache::Clean() {
}

/** Relation **/
Relation::~Relation() {
}

/** Collection **/
Collection::~Collection() {
}

/** Binding **/
Binding::~Binding() {
}