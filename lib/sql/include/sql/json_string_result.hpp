#ifndef LIB_SQL_JSON_STRING_RESULT_H_
#define LIB_SQL_JSON_STRING_RESULT_H_

#include "sql/concepts.hpp"
#include "sql/forward.hpp"
#include <sstream>
#include <string>

namespace sql
{
	/**
	 * Object to pass to `sqlite3_exec` function.
	 */
	class JsonStringResult
	{
	public:
		/**
		 * Callback to pass to `sqlite3_exec` function.
		 */
		static int callback(void* obj, int argc, char** argv, char** col_name);

		auto row(int column_count) -> void;

		auto column(
			std::string_view name,
			Type type,
			std::string_view value
		) -> void;

		/**
		 * Returns result as json array.
		 */
		auto get_array_result() -> std::string;
		
		/**
		 * Returns result as json object.
		 */
		auto get_object_result() -> std::string;

	private:
		std::stringstream m_stream;

		auto close_braces_if_needed() -> void;
	};
}

static_assert(sql::has_db_callback<sql::JsonStringResult>);
static_assert(sql::can_be_used_by_stmt<sql::JsonStringResult>);

#endif // LIB_SQL_JSON_STRING_RESULT_H_
