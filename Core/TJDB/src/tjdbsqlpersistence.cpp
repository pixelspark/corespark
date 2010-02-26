#include "../include/tjdbsqlpersistence.h"
using namespace tj::shared;
using namespace tj::db;

// TODO:
// - Embedded objects (struct Date; not as separate table through a rel but bindings embedded in parent object)
// - ToMany/ToSet relations
// - Inverse relations

/** SQLEntityContext **/
SQLEntityContext::SQLEntityContext(strong<Database> db, bool ro, bool async): _db(db), _readOnly(ro), _async(async) {
}

SQLEntityContext::~SQLEntityContext() {
}

ID SQLEntityContext::Add(strong<Entity> so, const Schema& schema, const EntityType& subType) {
	// Add super-entity first
	EntityType ultimateSubType = (subType.length()==0) ? schema.GetEntityType() : subType;
	ID super = Entity::UnknownObjectID;
	if(schema.IsSubSchema()) {
		super = Add(so, *(schema.GetSuperSchema()), ultimateSubType);
	}

	try {
		const std::set<Key>& keys = schema.GetKeys();

		std::wostringstream wos;
		wos << L"INSERT INTO " << schema.GetEntityType() << L" (super, subtype ";
		std::set<Key>::const_iterator it = keys.begin();
		while(it!=keys.end()) {
			Binding& bind = schema.GetBindingForKey(*it);
			if(!bind.IsCollection()) {
				wos << L", ";
				wos << (*it);
			}
			++it;
		}

		wos << L") VALUES (:super, :subtype";
		it = keys.begin();
		
		while(it!=keys.end()) {
			Binding& bind = schema.GetBindingForKey(*it);
			if(!bind.IsCollection()) {
				wos << L", ";
				wos << L":" << (*it);
			}
			++it;
		}
		wos << L");";

		ref<Query> q = _db->CreateQuery(wos.str());
		if(q) {
			q->Set(L"super", super);
			q->Set(L"subtype", subType);

			it = keys.begin();
			while(it!=keys.end()) {
				Binding& bind = schema.GetBindingForKey(*it);

				if(bind.IsCollection()) {
				}
				else if(bind.IsRelation()) {
					Relation& relation = bind.GetRelation(so);
					if(!relation.GetContext()) {
						relation.SetContext(ref<EntityContext>(this));
					}
					if(relation.GetContext()==ref<EntityContext>(this)) {
						relation.Commit();
						q->Set(*it, relation.GetID());
					}
					else {
						q->Set(*it, Entity::UnknownObjectID);
					}
				}
				else {
					q->Set(*it, bind.Get(so));
				}
				++it;
			}

			// Add this slice of the entity to the database
			q->Execute();
			ID insertedID = q->GetInsertedRowID();
			{
				strong<EntityCache> cache = schema.GetCache(ref<EntityContext>(this));
				ThreadLock lock(&cache->_lock);
				cache->PutEntity(insertedID, so);
			}

			// Set the inserted ID as parent ID on all contained collections
			it = keys.begin();
			while(it!=keys.end()) {
				Binding& bind = schema.GetBindingForKey(*it);
				if(bind.IsCollection()) {
					Collection& col = bind.GetCollection(so);
					col.Set(GetCollectionType(schema, *(bind.GetReferencedSchema()), *it), insertedID, ref<EntityContext>(this));
				}
				++it;
			}

			return insertedID;
		}
		Throw(L"EntityContext::Add: Could not create query!", ExceptionTypeError);
	}
	catch(const Exception& e) {
		Log::Write(L"TJDB/EntityContext", L"Could not add stored object to database: "+e.GetMsg());
		Throw(L"Could not add stored object", ExceptionTypeError);
	}
}

ID SQLEntityContext::Add(strong<Entity> so) {
	if(_readOnly) {
		return Entity::NewObjectID;
	}

	Transaction tr(_db);
	ID i = Add(so, so->GetSchema(), L"");
	tr.Commit();
	return i;
}

void SQLEntityContext::Update(const ID& i, strong<Entity> et) {
	Update(i, et, _async);
}

void SQLEntityContext::Update(const ID& i, strong<Entity> et, const Schema& schema) {
	const std::set<Key>& keys = schema.GetKeys();

	std::wostringstream wos;
	wos << L"UPDATE " << schema.GetEntityType() << L" SET ";
	bool omitComma = true;
	std::set<Key>::const_iterator it = keys.begin();
	while(it!=keys.end()) {
		Binding& bind = schema.GetBindingForKey(*it);
		if(!bind.IsCollection()) {
			if(!omitComma) {
				wos << L", ";
			}
			else {
				omitComma = false;
			}
			wos << (*it) << L" = :" << (*it);
		}
		++it;
	}

	if(schema.IsSubSchema()) {
		wos << L" WHERE super=:id;";
	}
	else {
		wos << L" WHERE id=:id;";
	}

	ref<Query> q = _db->CreateQuery(wos.str());
	if(q) {
		q->Set(L"id", i);
		it = keys.begin();
		while(it!=keys.end()) {
			Binding& bind = schema.GetBindingForKey(*it);
			if(bind.IsCollection()) {
				// Do not do anything
			}
			else if(bind.IsRelation()) {
				Relation& relation = bind.GetRelation(et);
				if(!relation.GetContext()) {
					relation.SetContext(ref<EntityContext>(this));
				}

				if(relation.GetContext()==ref<EntityContext>(this)) {
					relation.Commit();
					q->Set(*it, relation.GetID());
				}
				else {
					// Ignore relation, the relation references an object in a different context
					q->Set(*it, Entity::UnknownObjectID);
				}
			}
			else {
				q->Set(*it, bind.Get(et));
			}
			++it;
		}

		q->Execute();
	}

	const Schema* superSchema = schema.GetSuperSchema();
	if(superSchema!=0) {
		Update(i, et, *superSchema);
	}
}

// Task to dispatch updates in SQLEntityContext
namespace tj {
	namespace db {
		class SQLUpdateEntityTask: public Task {
			public:
				SQLUpdateEntityTask(const ID& i, strong<Entity> et, strong<SQLEntityContext> ec): _id(i), _entity(et), _ctx(ec) {
				}
				
				virtual ~SQLUpdateEntityTask() {
				}
				
				virtual void Run() {
					_ctx->Update(_id, _entity, false);
				}
				
			protected:
				strong<SQLEntityContext> _ctx;
				ID _id;
				strong<Entity> _entity;
		};
	}
}

// Note that the ID is for the subtype (i.e. schema returned by et->GetSchema).
void SQLEntityContext::Update(const ID& i, strong<Entity> et, bool async) {
	if(_readOnly) {
		return;
	}

	// Queue dispatched task to call this method again with async==false
	if(async) {
		Dispatcher::CurrentOrDefaultInstance()->Dispatch(ref<Task>(GC::Hold(new SQLUpdateEntityTask(i, et, ref<SQLEntityContext>(this)))));
	}
	else {
		Transaction tr(_db);
		const Schema& schema = et->GetSchema();
		{
			ThreadLock lock(&(et->_lock));
			Update(i, et, schema);
		}
		tr.Commit();
	}
}

ref<Query> SQLEntityContext::RetrieveData(const Schema& schema, const ID& i) {
	const EntityType& type = schema.GetEntityType();
	std::set<Key> keys = schema.GetKeys();
	
	std::wostringstream wos;
	wos << L"SELECT subtype, super";
	std::set<Key>::const_iterator it = keys.begin();
	while(it!=keys.end()) {
		Binding& bind = schema.GetBindingForKey(*it);
		if(!bind.IsCollection()) {
			wos << L", ";
			wos << *it;
		}
		++it;
	}
	wos << L" FROM " << type;
	if(schema.IsSubSchema()) {
		wos << L" WHERE super=:id;";
	}
	else {
		wos << L" WHERE id=:id;";
	}
	
	ref<Query> q = _db->CreateQuery(wos.str());
	if(q) {
		q->Set(L"id", i);
		q->Execute();
		return q;
	}
	return null;
}

CollectionType SQLEntityContext::GetCollectionType(const Schema& from, const Schema& to, const Key& k) {
	std::wostringstream wos;
	wos << from.GetEntityType() << L"_" << k << L"_" << to.GetEntityType();
	return wos.str();
}

void SQLEntityContext::MaterializeWithData(ref<Query> q, ref<Entity> object, const Schema& schema, const ID& i) {
	const std::set<Key>& keys = schema.GetKeys();
	std::set<Key>::const_iterator it = keys.begin();
	int idx = 2;
	while(it!=keys.end()) {
		Binding& bind = schema.GetBindingForKey(*it);
		// TODO: check column type and force the any to the specific type
		if(bind.IsRelation()) {
			Relation& ri = bind.GetRelation(object);
			ri.Set(q->GetInt64(idx), ref<EntityContext>(this));
		}
		else if(bind.IsCollection()) {
			Collection& ci = bind.GetCollection(object);
			ci.Set(GetCollectionType(schema, *(bind.GetReferencedSchema()), *it), i, ref<EntityContext>(this));
		}
		else {
			bind.Set(object, Any(q->GetText(idx)));
		}
		++idx;
		++it;
	}
}

ref<Entity> SQLEntityContext::Materialize(const Schema& schema, const ID& i) {
	const EntityType& type = schema.GetEntityType();
	if(type.length()==0) {
		Throw(L"Please set object type for object schema before attempting to materialize an object!", ExceptionTypeError);
	}
	strong<EntityCache> cache = schema.GetCache(ref<EntityContext>(this));
	ThreadLock cachesLock(&_cacheLock);
	{
		ThreadLock lock(&cache->_lock);
		cache->Clean();
		ref<Entity> object = cache->GetEntity(i);
		if(object) {
			return object;
		}
	}

	// Retrieve object from database
	Transaction tr(_db);
	{
		ref<Query> q = RetrieveData(schema, i);
		if(q && q->HasRow()) {
			// Find out the subtype; if there is a subtype, find the schema, create the appropriate entity
			EntityType et = q->GetText(0);
			ref<Entity> object;
			if(et!=L"") {
				std::map<EntityType, const Schema*>::iterator sit = EntityContext::_schemas->find(et);
				if(sit==EntityContext::_schemas->end()) {
					Throw(L"Entity type unknown; please register it first through EntityContext::RegisterSchema!", ExceptionTypeError);
				}

				// Work from the subtype up to the super type to materialize
				const Schema& subSchema = *(sit->second);
				const Schema* currentSchema = sit->second;
				object = subSchema.CreateEntity();

				while(currentSchema!=0) {
					ref<Query> data = RetrieveData(*currentSchema, i);
					MaterializeWithData(data, object, *currentSchema, i);

					// Put the object in all object caches
					strong<EntityCache> cache = schema.GetCache(ref<EntityContext>(this));
					ThreadLock lock(&cache->_lock);
					cache->PutEntity(i, object);

					// Move up in the type hierarchy
					currentSchema = currentSchema->GetSuperSchema();
				}
			}
			else {
				object = schema.CreateEntity();
				MaterializeWithData(q, object, schema, i);
				cache->PutEntity(i, object);
			}

			object->OnMaterialized();
			return object;
		}
	}
	return null;
}

void SQLEntityContext::UpdateDatabaseSchema(const Schema& schema) {
	if(_readOnly) {
		return; // SELECT statements on older or non-existant versions of a table will simply fail
	}

	if(_goodSchemas.find(schema.GetEntityType())!=_goodSchemas.end()) {
		return;
	}

	Transaction tr(_db);
	{
		// Update super schema first
		if(schema.IsSubSchema()) {
			UpdateDatabaseSchema(*(schema.GetSuperSchema()));
		}

		ref<Query> s = _db->CreateQuery(L"SELECT name FROM sqlite_master WHERE type='table' AND name=:ot;");
		s->Set(L"ot", schema.GetEntityType());
		s->Execute();
		if(s->HasRow()) {
			// Table already exists; for now, assume it is the same
			// TODO: check columns, types; if changed, recreate table and copy all stuff over
		}
		else {
			const std::set<Key>& keys = schema.GetKeys();

			// Create main entity table
			std::wostringstream wos;
			wos << L"CREATE TABLE " << schema.GetEntityType() << L" (";
			if(!schema.IsSubSchema()) {
				wos << L"id INTEGER PRIMARY KEY, super INTEGER, ";
			}
			else {
				const Schema* super = schema.GetSuperSchema();
				wos << L"super INTEGER PRIMARY KEY CONSTRAINT fk_super REFERENCES " << super->GetEntityType() << L" ON DELETE CASCADE, ";
			}

			wos << L"subtype VARIANT";
			 
			std::map<Key, Binding*> collections;
			std::set<Key>::const_iterator it = keys.begin();

			while(it!=keys.end()) {
				Binding& bind = schema.GetBindingForKey(*it);
				if(bind.IsCollection()) {
					// No extra column, but build a table for this collection relation
					collections[*it] = &bind;
				}
				else if(bind.IsRelation()) {
					wos << L", ";
					const Schema* referenced = bind.GetReferencedSchema();
					if(referenced==0) {
						Throw(L"Relation binding found, but no schema referenced", ExceptionTypeError);
					}
					wos << (*it) << L" INTEGER CONSTRAINT fk_" << (*it) << L" REFERENCES " << referenced->GetEntityType();
					if(referenced->IsSubSchema()) {
						wos << L"(super) ";
					}
					else {
						wos << L"(id) ";
					}
					wos << L"ON DELETE SET NULL";
				}
				else {
					wos << L", ";
					wos << (*it) << L" VARIANT";
				}
				++it;
			}
			
			if(schema.IsSubSchema()) {
				wos << L", UNIQUE(super)";
			}

			wos << L");";

			ref<Query> q = _db->CreateQuery(wos.str());
			if(q) {
				q->Execute();
			}

			// Build collection tables
			if(collections.size()>0) {
				std::map<Key, Binding*>::iterator it = collections.begin();
				while(it!=collections.end()) {
					Binding* bind = it->second;
					const Schema* referenced = bind->GetReferencedSchema();
					if(referenced!=0) {
						std::wostringstream wcos;

						wcos << L"CREATE TABLE " << schema.GetEntityType() << L"_"+(it->first)+L"_" << referenced->GetEntityType() << L"(";
						wcos << L"id INTEGER CONSTRAINT fk_id REFERENCES " << schema.GetEntityType() << L" (";
						if(schema.IsSubSchema()) {
							wcos << L"super";
						}
						else {
							wcos << L"id";
						}
						wcos << L") ON DELETE CASCADE, ";

						wcos << L"toid INTEGER CONSTRAINT fk_to REFERENCES " << referenced->GetEntityType() << L" (";
						if(referenced->IsSubSchema()) {
							wcos << L"super";
						}
						else {
							wcos << L"id";
						}
						wcos << L") ON DELETE CASCADE);";
						ref<Query> qc = _db->CreateQuery(wcos.str());
						if(qc) {
							qc->Execute();
						}
					}

					++it;
				}
			}
		}

		_goodSchemas.insert(schema.GetEntityType());
	}
	tr.Commit();
}

void SQLEntityContext::Remove(const ID& i, const Schema& schema) {
	Remove(i, schema, true);
}

void SQLEntityContext::Remove(const ID& i, const Schema& schema, bool transaction) {
	// TODO: find all referring relations and set those to Entity::UnknownObjectID
	// i.e. UPDATE person SET somerel=-3 WHERE somerel=:id
	// This probably also has to set the id's in the rel's of the cached objects to UnknownObjectID... (or maybe
	// RemovedObjectID).
	if(_readOnly) {
		return;
	}

	Transaction tr(_db);

	try {
		std::wostringstream wos;
		wos << L"DELETE FROM " << schema.GetEntityType();
		if(schema.IsSubSchema()) {
			wos << L" WHERE super=:id;";
		}
		else {
			wos << L" WHERE id=:id;";
		}

		ref<Query> q = _db->CreateQuery(wos.str());
		q->Set(L"id",  i);
		q->Execute();

		const Schema* superSchema = schema.GetSuperSchema();
		if(superSchema!=0) {
			Remove(i, *superSchema, false);
		}
		tr.Commit();
	}
	catch(const Exception& e) {
		Log::Write(L"TJDB/EntityContext", L"Could not add stored object to database: "+e.GetMsg());
		Throw(L"Could not add stored object", ExceptionTypeError);
	}
}

int64 SQLEntityContext::GetCollectionSize(const CollectionType& ct, const ID& i) {
	std::wostringstream wos;
	wos << L"SELECT COUNT(toid) FROM " << ct << L" WHERE id=:id;";

	Transaction tr(_db);
	int64 n = 0;
	{
		ref<Query> q = _db->CreateQuery(wos.str());
		q->Set(L"id", i);
		q->Execute();
		if(q->HasRow()) {
			n = q->GetInt64(0);
		}
	}
	tr.Commit();
	return n;
}

void SQLEntityContext::AddToCollection(const CollectionType& ct, const ID& from, const ID& toid) {
	std::wostringstream wos;
	wos << L"INSERT INTO " << ct << L" (id, toid) VALUES (:id, :toid);";

	Transaction tr(_db);
	{
		ref<Query> q = _db->CreateQuery(wos.str());
		q->Set(L"id", from);
		q->Set(L"toid", toid);
		q->Execute();
	}
	tr.Commit();
}

void SQLEntityContext::RemoveFromCollection(const CollectionType& ct, const Schema& referenced, const ID& i, const ID& toid) {
	std::wostringstream wos;
	wos << L"DELETE FROM " << ct << L" WHERE (id=:id AND toid=:toid);";

	Transaction tr(_db);
	{
		ref<Query> q = _db->CreateQuery(wos.str());
		q->Set(L"id", i);
		q->Set(L"toid", toid);
		q->Execute();
	}
	tr.Commit();
}

ID SQLEntityContext::GetCollectionItem(const CollectionType& ct, const Schema& referenced, const ID& i, int64 idx) {
	std::wostringstream wos;
	wos << L"SELECT " << ct << ".toid FROM " << ct << L" LIMIT 1 OFFSET " << idx << L";";

	ID toid = Entity::UnknownObjectID;
	Transaction tr(_db);
	{
		ref<Query> q = _db->CreateQuery(wos.str());
		q->Execute();
		if(q->HasRow()) {
			toid = q->GetInt64(0);
		}
	}
	tr.Commit();

	return toid;
}
