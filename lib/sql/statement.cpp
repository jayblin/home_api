#include "sql/statement.hpp"
#include <iostream>
#include <string>

sql::Statement& sql::Statement::exec()
{
	auto rc = sqlite3_step(m_stmt);
	m_status = static_cast<status::Code>(rc);

	return *this;
}

// @todo Определять динамически, когда использовать SQLITE_TRANSIENT,
// а когда SQLITE_STATIC.
/* sql::Statement& sql::Statement::bind(int idx, const std::string& value, sql::Type t) */
sql::Statement& sql::Statement::bind(int idx, std::string_view value, sql::Type t)
{
	int rc = 0;

	switch (t)
	{
		case Type::SQL_TEXT:
			rc = sqlite3_bind_text(
				m_stmt, idx, value.data(), value.size(), SQLITE_TRANSIENT
			);
			break;
		case Type::SQL_DOUBLE:
			rc = sqlite3_bind_double(m_stmt, idx, std::stod(value.data()));
			break;
		case Type::SQL_BLOB:
			rc = sqlite3_bind_blob(
				m_stmt, idx, value.data(), value.size(), SQLITE_TRANSIENT
			);
			break;
		case Type::SQL_INT:
			rc = sqlite3_bind_int(m_stmt, idx, std::stoi(value.data()));
			break;
		case Type::SQL_NULL:
			rc = sqlite3_bind_null(m_stmt, idx);
			break;
	}

	m_status = static_cast<status::Code>(rc);

	return *this;
}

std::string sql::Statement::column_value(Type type, int column_idx)
{
	switch (type)
	{
		case Type::SQL_INT: {
			std::ostringstream ss;

			ss << sqlite3_column_int(m_stmt, column_idx);

			return ss.str();
		}
		case Type::SQL_DOUBLE: {
			std::ostringstream ss;

			ss << sqlite3_column_double(m_stmt, column_idx);

			return ss.str();
	   }
		case Type::SQL_TEXT: {
			const std::string::size_type size = sqlite3_column_bytes(m_stmt, column_idx);
			return {
				reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, column_idx)),
				size
			};
		}
		case Type::SQL_BLOB: {
			const std::string::size_type size = sqlite3_column_bytes(m_stmt, column_idx);
			return {
				static_cast<const char*>(sqlite3_column_blob(m_stmt, column_idx)),
				size
			};
		}
		case Type::SQL_NULL:
			return "";
		default:
			return "";
	}
}
