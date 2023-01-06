#include "tables/utils.hpp"
#include "tables/concepts.hpp"
#include "tables/definitions.hpp"

static std::string remove_quotes(const std::string& str)
{
	if (str.length() > 1 && '"' == str[0] && '"' == str[str.length() - 1])
	{
		return str.substr(1, str.length() - 2);
	}

	return str;
}

static std::string get_value(
	const tables::Column& column,
	const nlohmann::json& data
)
{
	using T = sql::Type;

	if (data.find(column.name) == data.end() || data[column.name].is_null())
	{
		switch (column.datatype)
		{
			case T::SQL_TEXT:
				return "";
			case T::SQL_DOUBLE:
				return "0.0";
			case T::SQL_INT:
				return "0";
			default:
				return "";
		}
	}

	return remove_quotes(data[column.name].dump());

	auto value = data[column.name].dump();
	value = remove_quotes(value);

	return value;
}

std::string tables::generate_insert_clause(const tables::Definition& definition)
{
	std::stringstream ss;
	ss << "INSERT INTO " << definition.table_name << " (";

	bool skip = true;
	for (const auto& column : definition.columns)
	{
		if (skip)
		{
			ss << column.name;

			skip = false;
			continue;
		}

		ss << "," << column.name;
	}

	ss << ") values (";
	skip = true;
	for (const auto& column : definition.columns)
	{
		if (skip)
		{
			ss << '?';

			skip = false;
			continue;
		}

		ss << ",?";
	}
	ss << ')';

	return ss.str();
}

std::vector<std::string> tables::get_constraints_errors(
	const tables::Definition& definition,
	const nlohmann::json& data
)
{
	using C = Constraint;

	std::vector<std::string> errors;

	for (const Column& column : definition.columns)
	{
		if (data.find(column.name) == data.end())
		{
			continue;
		}

		const auto& datum = data[column.name].dump();

		// @todo: handle each dadatype.
		switch (column.datatype)
		{
			case sql::Type::SQL_TEXT:

				for (const auto constraint : column.constraints)
				{
					if (constraint == Constraint::NOT_EMPTY)
					{
						if (datum.length() == 0 || (
							datum.length() == 2 && datum[0] == '"' && datum[1] == '"'
						))
						{
							errors.emplace_back(
								std::string{column.description} + " не должно быть пустым"
							);
						}
					}
				}
				break;
		}
	}

	return errors;
}

void tables::bind_values(
	const tables::Definition& definition,
	sql::Statement& stmt,
	const nlohmann::json& data
)
{
	int idx = 0;

	for (const Column& column : definition.columns)
	{
		stmt.bind(
			++idx,
			get_value(column, data),
			column.datatype
		);
	}
}
