#ifndef _TABLES_DEFINITIONS_H_
#define _TABLES_DEFINITIONS_H_

#include "concepts.hpp"
#include <array>
#include <string_view>

namespace tables
{
	struct Definition
	{
		const std::string_view table_name;
		const std::vector<Column> columns;
	};

	const std::vector<Definition>& definitions();
}

#endif // _TABLES_DEFINITIONS_H_
