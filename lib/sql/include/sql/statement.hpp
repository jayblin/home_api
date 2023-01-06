#ifndef LIB_SQL_STATEMENT_H_
#define LIB_SQL_STATEMENT_H_

#include "sql/concepts.hpp"
#include "sql/sqlite3.hpp"
#include "sqlite3.h"
#include "sql/forward.hpp"
#include <sstream>
#include <string>
#include <string_view>
#include <iostream>

namespace sql
{
	class Statement
	{
	public:
		Statement(const Sqlite3& db, const std::string& sql)
		{
			auto rc = sqlite3_prepare_v2(
				db.m_db, sql.data(), sql.size(), &m_stmt, nullptr
			);
			
			m_status = static_cast<status::Code>(rc);
		}

		~Statement()
		{
			sqlite3_finalize(m_stmt);
		}

		/**
		 * Returns the value of a column.
		 */
		auto column_value(Type type, int column_idx) -> std::string;

		/**
		 * Binds a value to argument at position `idx`.
		 */
		/* auto bind(int idx, const std::string& value, Type t) -> Statement&; */
		auto bind(int idx, std::string_view value, Type t) -> Statement&;

		/**
		 * Executes the statement once.
		 * Usefull for ISNERT/UPDATE/DELETE.
		 */
		auto exec() -> Statement&;

		/**
		 * Executes statement and returns an object of `T`, that must be able to
		 * handle data that is received from database.
		 * Useful for SELECT.
		 */
		template <class T>
			requires can_be_used_by_stmt<T>
		auto exec() -> T;

		constexpr auto status() const -> const status::Code { return m_status; }

	private:
		sqlite3_stmt* m_stmt = nullptr;
		status::Code m_status = status::Code::OK;
	};

	template <class T>
		requires can_be_used_by_stmt<T>
	T Statement::exec()
	{
		T obj {};

		int _i = 0;
		do
		{
			++_i;
			exec();

			const auto col_count = sqlite3_data_count(m_stmt);

			if (status::Code::DONE == m_status || 0 == col_count)
			{
				break;
			}

			obj.row(col_count);

			for (auto i = 0; i < col_count; i++)
			{
				const auto t = static_cast<Type>(sqlite3_column_type(m_stmt, i));
				obj.column(
					sqlite3_column_name(m_stmt, i),
					t,
					column_value(t, i)
				);
			}
		}
		while (status::Code::ROW == m_status  && _i < 256);

		return obj;
	}
}

#endif // LIB_SQL_STATEMENT_H_
