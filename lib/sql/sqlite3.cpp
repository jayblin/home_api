#include "sql/sqlite3.hpp"
#include "sql/statement.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

sql::Sqlite3::Sqlite3(std::string_view&& file_name)
{
	auto rc = sqlite3_open(file_name.data(), &m_db);

	if (rc)
	{
		sqlite3_close(m_db);
	}
}

sql::Sqlite3::~Sqlite3()
{
	sqlite3_close(m_db);
}

void sql::Sqlite3::exec(
    std::string_view&& sql,
    sqlite3_callback callback,
    void* obj
)
{
	auto rc = sqlite3_exec(m_db, sql.data(), callback, obj, &m_err_msg);

	m_status = static_cast<status::Code>(rc);
}
