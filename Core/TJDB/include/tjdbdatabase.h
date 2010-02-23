#ifndef _TJDBDATABASE_H
#define _TJDBDATABASE_H

#include "internal/tjdbinternal.h"
#pragma warning(push)
#pragma warning(disable: 4251 4275)

namespace tj {
	namespace db {
		typedef std::wstring Table;
		class Query;

		class DB_EXPORTED Database: public virtual tj::shared::Object {
			public:
				virtual ~Database();
				virtual void GetTables(std::vector<Table>& lst) = 0;
				virtual ref<Query> CreateQuery(const std::wstring& sql) = 0;
				virtual std::wstring GetVersion() = 0;
				virtual std::wstring GetName() = 0;
				virtual void BeginTransaction() = 0;
				virtual void CommitTransaction() = 0;
				virtual void RollbackTransaction() = 0;
				virtual bool IsInTransaction() const = 0;

				static ref<Database> Open(const std::wstring& file, bool strictlyTransactional = false);

				tj::shared::CriticalSection _lock;
		};

		class DB_EXPORTED Transaction {
			public:
				Transaction(tj::shared::strong<Database> db);
				~Transaction();
				void Commit();

			protected:
				tj::shared::strong<Database> _db;
				tj::shared::ThreadLock _lock;
				bool _committed;
		};

		class DB_EXPORTED Query: public virtual tj::shared::Object {
			public:
				virtual ~Query();

				// Parameter binding
				virtual void Set(const std::wstring& param, const std::wstring& str) = 0;
				virtual void Set(const std::wstring& param, int i) = 0;
				virtual void Set(const std::wstring& param, double v) = 0;
				virtual void Set(const std::wstring& param, bool t) = 0;
				virtual void Set(const std::wstring& param, tj::shared::int64 i) = 0;
				virtual void Set(const std::wstring& param, const tj::shared::Any& val);

				virtual void Set(int param, const std::wstring& str) = 0;
				virtual void Set(int param, int i) = 0;
				virtual void Set(int param, double v) = 0;
				virtual void Set(int, bool t) = 0;
				virtual void Set(int, tj::shared::int64 i) = 0;
				virtual void Set(int, const tj::shared::Any& val);

				// Execution and fetching result
				virtual tj::shared::int64 GetInsertedRowID() = 0;
				virtual void Reset() = 0;
				virtual void Execute() = 0;
				virtual bool HasRow() = 0;
				virtual void Next() = 0;
				virtual unsigned int GetColumnCount() = 0;
				virtual std::wstring GetColumnName(int col) = 0;

				// Getting data
				virtual int GetInt(int col) = 0;
				virtual std::wstring GetText(int col) = 0;
				virtual tj::shared::int64 GetInt64(int col) = 0;
				virtual bool GetBool(int col) = 0;
				virtual double GetDouble(int col) = 0;
				virtual tj::shared::Any GetAny(int col) = 0;

		};
	}
}

#pragma warning(pop)
#endif