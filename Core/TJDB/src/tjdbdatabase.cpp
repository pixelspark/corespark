#include "../include/tjdb.h"
#include <SQLite/sqlite3.h>
using namespace tj::db;
using namespace tj::shared;

Copyright KCRSQLite(L"TJDB", L"SQLite", L"(public domain software)");

/** Transaction **/
Transaction::Transaction(strong<Database> db): _db(db), _lock(&(db->_lock)), _committed(false) {
	_db->BeginTransaction();
}

Transaction::~Transaction() {
	if(!_committed) {
		_db->RollbackTransaction();
	}
}

void Transaction::Commit() {
	_db->CommitTransaction();
	_committed = true;
}

/* SQLiteDatabase */
namespace tj {
	namespace db {
		namespace sqlite {
			class SQLiteInitializer {
				public:
					SQLiteInitializer() {
						sqlite3_initialize();
					}

					~SQLiteInitializer() {
						sqlite3_shutdown();
					}
			};

			SQLiteInitializer _sqliteInitializer;

			class SQLiteDatabase: public Database {
				friend class SQLiteQuery;

				public:
					SQLiteDatabase(const std::wstring& path, bool strictlyTransactional = false);
					virtual ~SQLiteDatabase();
					virtual void GetTables(std::vector<Table>& list);
					virtual ref<Query> CreateQuery(const std::wstring& sql);
					virtual std::wstring GetVersion();
					virtual std::wstring GetName();
					virtual void BeginTransaction();
					virtual void CommitTransaction();
					virtual void RollbackTransaction();
					virtual bool IsInTransaction() const;

				protected:
					void Error();
					sqlite3* _db;
					int _transactionCount;
					bool _strictlyTransactional;
			};

			class SQLiteQuery: public Query {
				public:
					SQLiteQuery(ref<SQLiteDatabase> db, const std::wstring& sql);
					virtual ~SQLiteQuery();

					virtual void Set(const std::wstring& param, const std::wstring& str);
					virtual void Set(const std::wstring& param, int i);
					virtual void Set(const std::wstring& param, double v);
					virtual void Set(const std::wstring& param, bool t);
					virtual void Set(const std::wstring& param, tj::shared::int64 i);

					virtual void Set(int param, const std::wstring& str);
					virtual void Set(int param, int i);
					virtual void Set(int param, double v);
					virtual void Set(int, bool t);
					virtual void Set(int, tj::shared::int64 i);

					virtual void Reset();
					virtual void Execute();
					virtual bool HasRow();
					virtual void Next();
					virtual unsigned int GetColumnCount();
					virtual tj::shared::int64 GetInsertedRowID();
					virtual std::wstring GetColumnName(int col);

					virtual int GetInt(int col);
					virtual std::wstring GetText(int col);
					virtual tj::shared::int64 GetInt64(int col);
					virtual bool GetBool(int col);
					virtual double GetDouble(int col);
					virtual Any GetAny(int col);

				protected:
					ref<SQLiteDatabase> _db;
					sqlite3_stmt* _st;
					bool _hasRow;
					
			};
		}
	}
}

using namespace tj::db::sqlite;

SQLiteDatabase::SQLiteDatabase(const std::wstring& path, bool strictlyTransactional): _transactionCount(0), _strictlyTransactional(strictlyTransactional) {
	_db = 0;
	if(sqlite3_open(Mbs(path).c_str(), &_db)!=0) {
		throw Exception(L"Could not open database (path: '"+path+L"')", ExceptionTypeError, __FILE__, __LINE__);
	}
}

bool SQLiteDatabase::IsInTransaction() const {
	return _transactionCount > 0;
}

SQLiteDatabase::~SQLiteDatabase() {
	if(sqlite3_exec(_db, "VACUUM;", NULL, NULL, NULL)!=0) {
		Log::Write(L"TJDB/SQLiteDatabase", L"Could not vacuum database");
	}

	if(sqlite3_close(_db)!=0) {
		// Could not close, but we cannot do anything about it here
		Log::Write(L"TJDB/SQLiteDatabase", L"Could not close database; losing data?");
	}
}

std::wstring SQLiteDatabase::GetName() {
	return L"SQLite";
}

std::wstring SQLiteDatabase::GetVersion() {
	return Wcs(SQLITE_VERSION);
}

void SQLiteDatabase::Error() {
	if(sqlite3_errcode(_db)!=0) {
		const wchar_t* message = reinterpret_cast<const wchar_t*>(sqlite3_errmsg16(_db));
		Throw(message, ExceptionTypeError);
	}
}

void SQLiteDatabase::GetTables(std::vector<Table>& lst) {
	ref<Query> st = CreateQuery(L"SELECT name FROM sqlite_master WHERE type='table' ORDER BY name");
	st->Execute();
	
	while(st->HasRow()) {
		lst.push_back(st->GetText(0));
		st->Next();
	}
}

void SQLiteDatabase::BeginTransaction() {
	ThreadLock lock(&_lock);
	if(_transactionCount==0) {
		if(sqlite3_exec(_db, "BEGIN TRANSACTION;", NULL, NULL, NULL)!=0) {
			Error();
		}
	}
	++_transactionCount;
}


void SQLiteDatabase::CommitTransaction() {
	ThreadLock lock(&_lock);
	if(_transactionCount==1) {
		if(sqlite3_exec(_db, "COMMIT;", NULL, NULL, NULL)!=0) {
			Error();
		}
	}
	--_transactionCount;
}

void SQLiteDatabase::RollbackTransaction() {
	ThreadLock lock(&_lock);
	if(_transactionCount!=0) {
		if(sqlite3_exec(_db, "ROLLBACK;", NULL, NULL, NULL)!=0) {
			Error();
		}
		_transactionCount = 0;
	}
}

ref<Query> SQLiteDatabase::CreateQuery(const std::wstring& sql) {
	if(Zones::IsDebug()) {
		Log::Write(L"TJDB/SQLiteDatabase", sql);
	}
	return GC::Hold(new SQLiteQuery(this, sql));
}

/* SQLiteQuery */
SQLiteQuery::SQLiteQuery(ref<SQLiteDatabase> db, const std::wstring& sql): _db(db), _hasRow(false) {
	if(sqlite3_prepare16_v2(db->_db, (const void*)sql.c_str(), (int)sql.length()*sizeof(wchar_t), &_st, 0)!=0) {
		db->Error();
	}
}

SQLiteQuery::~SQLiteQuery() {
	sqlite3_finalize(_st);
}

void SQLiteQuery::Set(const std::wstring& param, const std::wstring& str) {
	std::string par = ':' + Mbs(param);
	int i = sqlite3_bind_parameter_index(_st, par.c_str());

	if(i==0) {
		_db->Error();
	}

	Set(i, str);
}

void SQLiteQuery::Set(const std::wstring& param, int i) {
	std::string par = ':' + Mbs(param);
	int p = sqlite3_bind_parameter_index(_st, par.c_str());
	if(p==0) {
		_db->Error();
	}

	Set(p,i);
}

void SQLiteQuery::Set(const std::wstring& param, double v) {
	std::string par = ':' + Mbs(param);
	int i = sqlite3_bind_parameter_index(_st, par.c_str());
	if(i==0) {
		_db->Error();
	}

	Set(i, v);
}

void SQLiteQuery::Set(const std::wstring& param, bool t) {
	std::string par = ':' + Mbs(param);
	int i = sqlite3_bind_parameter_index(_st, par.c_str());
	if(i==0) {
		_db->Error();
	}

	Set(i, t);
}

void SQLiteQuery::Set(const std::wstring& param, tj::shared::int64 v) {
	std::string par = ':' + Mbs(param);
	int i = sqlite3_bind_parameter_index(_st, par.c_str());
	if(i==0) {
		_db->Error();
	}

	Set(i, v);
}

void SQLiteQuery::Set(int param, const std::wstring& str) {
	if(sqlite3_bind_text16(_st, param, str.c_str(), int(str.length())*sizeof(wchar_t), SQLITE_TRANSIENT)!=0) {
		_db->Error();
	}
}

void SQLiteQuery::Set(int param, int i) {
	if(sqlite3_bind_int(_st, param, i)!=0) {
		_db->Error();
	}
}

void SQLiteQuery::Set(int param, double v) {
	if(sqlite3_bind_double(_st, param, v)!=0) {
		_db->Error();
	}
}

void SQLiteQuery::Set(int p, bool t) {
	Set(p, t ? 1 : 0);
}
void SQLiteQuery::Set(int p, tj::shared::int64 i) {
	if(sqlite3_bind_int64(_st, p, i)!=0) {
		_db->Error();
	}
}

unsigned int SQLiteQuery::GetColumnCount() {
	if(_hasRow) {
		int rn = sqlite3_column_count(_st);
		if(rn>0) {
			return (unsigned int)(rn);
		}
		return 0;
	}
	return 0; // Query without a result (DELETE, INSERT, etc.)
}

std::wstring SQLiteQuery::GetColumnName(int col) {
	const wchar_t* name = reinterpret_cast<const wchar_t*>(sqlite3_column_name16(_st, col));
	if(name==0) {
		return L"?";
	}
	return name;
}

void SQLiteQuery::Reset() {
	int r = sqlite3_reset(_st);
	_hasRow = false;
	if(r!=0) {
		_db->Error();
	}
}

int64 SQLiteQuery::GetInsertedRowID() {
	return sqlite3_last_insert_rowid(_db->_db);
}

void SQLiteQuery::Execute() {
	if(_db->_strictlyTransactional && !_db->IsInTransaction()) {
		Throw(L"Cannot execute queries outside a databae transaction; please wrap your query code inside a block containing a Transaction object", ExceptionTypeError);
	}
	int r = sqlite3_step(_st);
	
	if(r==SQLITE_ROW) {
		_hasRow = true;
	}
	else if(r==SQLITE_DONE) {
		_hasRow = false;
	}
	else {
		_db->Error();
	}
}

bool SQLiteQuery::HasRow() {
	return _hasRow;
}

void SQLiteQuery::Next() {
	int r = sqlite3_step(_st);

	if(r==SQLITE_DONE) {
		_hasRow = false;
	}
	else if(r==SQLITE_ROW) {
		_hasRow = true;
	}
	else {
		_db->Error();
	}
}

int SQLiteQuery::GetInt(int col) {
	if(!_hasRow) {
		Throw(L"Cannot fetch result data when there is no current row", ExceptionTypeError);
	}

	return sqlite3_column_int(_st, col);
}

std::wstring SQLiteQuery::GetText(int col) {
	if(!_hasRow) {
		Throw(L"Cannot fetch result data when there is no current row", ExceptionTypeError);
	}

	return std::wstring((const wchar_t*)sqlite3_column_text16(_st, col));
}

tj::shared::int64 SQLiteQuery::GetInt64(int col) {
	if(!_hasRow) {
		Throw(L"Cannot fetch result data when there is no current row", ExceptionTypeError);
	}

	return (tj::shared::int64)sqlite3_column_int64(_st, col);
}

bool SQLiteQuery::GetBool(int col) {
	return GetInt(col)==1;
}

Any SQLiteQuery::GetAny(int col) {
	int type = sqlite3_column_type(_st, col);
	switch(type) {
		case SQLITE_INTEGER:
			return Any(GetInt(col));

		case SQLITE_FLOAT:
			return Any(GetDouble(col));

		case SQLITE_TEXT:
		case SQLITE_BLOB:
			return Any(GetText(col));

		default:
		case SQLITE_NULL:
			return Any();
	}
}

double SQLiteQuery::GetDouble(int col) {
	if(!_hasRow) {
		Throw(L"Cannot fetch result data when there is no current row", ExceptionTypeError);
	}

	return sqlite3_column_double(_st, col);
}


/* Database */
Database::~Database() {
}

ref<Database> Database::Open(const std::wstring& path, bool str) {
	return GC::Hold(new SQLiteDatabase(path, str));
}

/* Query */
Query::~Query() {
}

void Query::Set(const std::wstring& param, const Any& val) {
	switch(val.GetType()) {
		case Any::TypeBool:
			Set(param, bool(val));
			break;

		case Any::TypeDouble:
			Set(param, double(val));
			break;

		case Any::TypeString:
			Set(param, String(val));
			break;

		case Any::TypeInteger:
			Set(param, int(val));
			break;
	}
}

void Query::Set(int param, const Any& val) {
	switch(val.GetType()) {
		case Any::TypeBool:
			Set(param, bool(val));
			break;

		case Any::TypeDouble:
			Set(param, double(val));
			break;

		case Any::TypeString:
			Set(param, String(val));
			break;

		case Any::TypeInteger:
			Set(param, int(val));
			break;
	}
}
