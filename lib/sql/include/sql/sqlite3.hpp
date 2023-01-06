#ifndef LIB_SQL_SQLITE3_H_
#define LIB_SQL_SQLITE3_H_

#include "sql/concepts.hpp"
#include "sqlite3.h"
#include <concepts>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include "sql/forward.hpp"

namespace sql
{
	/**
	 * A wrapper arond sqlite3 functions.
	 */
	class Sqlite3
	{
	public:
		Sqlite3(std::string_view&& file_name);
		~Sqlite3();

		/**
		 * Executes `sql` and invokes `callback` for each row.
		 */
		auto exec(
		    std::string_view&& sql,
		    sqlite3_callback callback = nullptr,
		    void* obj = nullptr
		) -> void;

		/**
		 * Executes `sql` and returns an object of `T`, that must be able to
		 * handle data that is received from database.
		 */
		template <class T>
			requires has_db_callback<T>
		auto exec(std::string_view&& sql) -> T;

		auto message() const -> const char* { return m_err_msg; };
		
		constexpr auto status() const -> const status::Code { return m_status; }

	private:
		sqlite3* m_db;
		char* m_err_msg = nullptr;
		status::Code m_status = status::Code::OK;

		friend Statement;
	};

	template <class T>
		requires has_db_callback<T>
	T Sqlite3::exec(std::string_view&& sql)
	{
		T obj {};
		auto rc = sqlite3_exec(
			m_db, sql.data(), obj.callback, &obj, &m_err_msg
		);

		m_status = static_cast<status::Code>(rc);

		return obj;
	}
} // namespace sql

#endif // LIB_SQL_SQLITE3_H_
