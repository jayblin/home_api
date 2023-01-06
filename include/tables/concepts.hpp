#ifndef TABLES_CONCEPTS_H_
#define TABLES_CONCEPTS_H_

#include "sql/forward.hpp"
#include <concepts>
#include <string_view>
#include <type_traits>
#include <vector>

namespace tables
{
	enum class Constraint
	{
		NOT_EMPTY,
		UNIQUE,
	};

	struct Column
	{
		std::string_view name;
		std::string_view description;
		sql::Type datatype;
		std::vector<Constraint> constraints;
	};
}

#endif // TABLES_CONCEPTS_H_

