#include "util/json.hpp"
#include <sstream>
#include <string>
#include <vector>

std::string util::json::remove_quotes(std::string_view str)
{
	if (str.length() > 1 && '"' == str[0] && '"' == str[str.length() - 1])
	{
		return std::string{str.substr(1, str.length() - 2)};
	}

	return std::string{str};
}

std::string
    util::json::create_errors_json(const std::vector<std::string>& errors)
{
	std::stringstream ss;

	ss << "{\"errors\":[";

	for (const auto& error : errors)
	{
		ss << "{\"detail\":\"" << error << "\"}";

		if (errors.back() != error)
		{
			ss << ',';
		}
	}

	ss << "]}";

	return ss.str();
}

/* std::string util::json::get_value( */
/*     constraint::column_name_t column_name, */
/*     sqlw::Type column_type, */
/*     const nlohmann::json& data */
/* ) */
/* { */
/* 	using T = sqlw::Type; */

/* 	if (data.find(column_name) == data.end() || data[column_name].is_null()) */
/* 	{ */
/* 		switch (column_type) */
/* 		{ */
/* 			case T::SQL_TEXT: */
/* 				return ""; */
/* 			case T::SQL_DOUBLE: */
/* 				return "0.0"; */
/* 			case T::SQL_INT: */
/* 				return "0"; */
/* 			default: */
/* 				return ""; */
/* 		} */
/* 	} */

/* 	return remove_quotes(data[column_name].dump()); */

/* 	auto value = data[column_name].dump(); */
/* 	value = remove_quotes(value); */

/* 	return value; */
/* } */

/* static void emplace_empty_value_error( */
/*     std::vector<std::string>& errors, */
/*     const std::string_view& column_name */
/* ) */
/* { */
/* 	errors.emplace_back( */
/* 	    "Поле " + std::string {column_name} + " не должно быть пустым" */
/* 	); */
/* } */


/* static void emplace_error_if_value_is_empty( */
/*     std::vector<std::string>& errors, */
/*     util::Constraint constraint, */
/*     const std::string_view& column_name, */
/*     const std::string& value */
/* ) */
/* { */
/* 	if (util::Constraint::NOT_EMPTY == constraint */
/* 	    && (value.length() == 0 */
/* 	        || (value.length() == 2 && value[0] == '"' && value[1] == '"'))) */
/* 	{ */
/* 		emplace_empty_value_error(errors, column_name); */
/* 	} */
/* } */

/* void util::emplace_constraints_errors( */
/*     std::vector<std::string>& errors, */
/*     const util::ConstraintErrorAggregator& aggregator, */
/*     const nlohmann::json& request_body */
/* ) */
/* { */
/* 	if (!request_body.contains(*aggregator.column_name)) */
/* 	{ */
/* 		if (std::ranges::any_of( */
/* 		        aggregator.constraints, */
/* 		        [](auto constraint) */
/* 		        { */
/* 			        return util::Constraint::NOT_EMPTY == constraint; */
/* 		        } */
/* 		    )) */
/* 		{ */
/* 			emplace_empty_value_error(errors, *aggregator.column_name); */
/* 		} */

/* 		return; */
/* 	} */

/* 	const auto column_value = request_body[*aggregator.column_name].dump(); */
/* 	const auto is_float = [&column_value]() */
/* 	{ */
/* 		bool has_delimiter = false; */

/* 		for (auto c : column_value) */
/* 		{ */
/* 			if (c == '.' || c == ',') */
/* 			{ */
/* 				if (has_delimiter) */
/* 				{ */
/* 					return false; */
/* 				} */

/* 				has_delimiter = true; */
/* 			} */

/* 			if (!std::isdigit(c)) */
/* 			{ */
/* 				return false; */
/* 			} */
/* 		} */

/* 		return true; */
/* 	}; */
/* 	const auto is_int = [&column_value]() */
/* 	{ */
/* 		for (auto c : column_value) */
/* 		{ */
/* 			if (!std::isdigit(c)) */
/* 			{ */
/* 				return false; */
/* 			} */
/* 		} */

/* 		return true; */
/* 	}; */

/* 	switch (*aggregator.column_type) */
/* 	{ */
/* 		case sqlw::Type::SQL_TEXT: */
/* 			for (const auto constraint : aggregator.constraints) */
/* 			{ */
/* 				emplace_error_if_value_is_empty( */
/* 				    errors, */
/* 				    constraint, */
/* 				    *aggregator.column_name, */
/* 				    column_value */
/* 				); */
/* 			} */

/* 			break; */
/* 		case sqlw::Type::SQL_DOUBLE: */
/* 			if (!is_float()) */
/* 			{ */
/* 				errors.emplace_back( */
/* 				    "Поле " + std::string {*aggregator.column_name} */
/* 				    + " должно быть числом с плавающей запятой" */
/* 				); */
/* 			} */
/* 			break; */
/* 		case sqlw::Type::SQL_INT: */
/* 			if (!is_int()) */
/* 			{ */
/* 				errors.emplace_back( */
/* 				    "Поле " + std::string {*aggregator.column_name} */
/* 				    + " должно быть целым числом" */
/* 				); */
/* 			} */
/* 			break; */
/* 	} */
/* } */
