#ifndef LIB_SQL_CONCEPTS_H_
#define LIB_SQL_CONCEPTS_H_

#include "sql/forward.hpp"
#include <concepts>
#include <string>

namespace sql
{
	template<class T>
	concept has_db_callback = requires(
		void* object,
		int argc,
		char** argv,
		char** column_name
	)
	{
		{ T::callback(object, argc, argv, column_name) } -> std::same_as<int>;
	};

	template<class T>
	concept can_be_used_by_stmt = requires(
		T t,
		std::string_view column_name,
		Type column_type,
		std::string_view column_value,
		int column_count
	)
	{
		{ t.row(column_count) } -> std::same_as<void>;
		{ t.column(column_name, column_type, column_value) } -> std::same_as<void>;
	};
}

#endif // LIB_SQL_CONCEPTS_H_
