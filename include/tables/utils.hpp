#ifndef TABLES_UTILS_H_
#define TABLES_UTILS_H_

#include "nlohmann/json.hpp"
#include "sql/forward.hpp"
#include "sql/statement.hpp"
#include "tables/concepts.hpp"
#include "tables/definitions.hpp"
#include <sstream>

namespace tables
{
	/**
	 * Creates an INSERT calse for a table that is defined by `definition`;
	 */
	auto generate_insert_clause(const Definition& definition) -> std::string;

	/**
	 * Returns a vector of messages that say why some elements of `data` are not
	 * correct fo corresponding columns of a table defined by `definition`.
	 */
	auto get_constraints_errors(
		const Definition& definition,
		const nlohmann::json& data
	) -> std::vector<std::string>;

	/**
	 * Binds values from `data` to arguments in `stmt` based on `definition`.
	 */
	auto bind_values(
		const Definition& definition,
		sql::Statement& stmt,
		const nlohmann::json& data
	) -> void;
}

#endif // TABLES_UTILS_H_

