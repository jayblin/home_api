#ifndef UTIL_JSON_H_
#define UTIL_JSON_H_

#include <string>
#include <vector>

namespace util::json
{
	/* std::string get_value( */
	/*     constraint::column_name_t column_name, */
	/*     sqlw::Type column_type, */
	/*     const nlohmann::json& data */
	/* ); */

	std::string remove_quotes(std::string_view str);

	std::string create_errors_json(const std::vector<std::string>& errors);

	/* void emplace_constraints_errors( */
	/*     std::vector<std::string>& errors, */
	/*     const constraint::ErrorAggregator& aggregator, */
	/*     const nlohmann::json& request_body */
	/* ); */

	/* template<typename T> */
	/* requires constraint::has_columns<T> */
	/* std::vector<std::string> */
	/*     get_constraint_errors(const nlohmann::json& request_body) */
	/* { */
	/* 	std::vector<std::string> errors; */

	/* 	std::apply( */
	/* 	    [&errors, &request_body](auto&&... columns) */
	/* 	    { */
	/* 		    ((std::apply( */
	/* 		         [&errors, &request_body](auto&&... args) */
	/* 		         { */
	/* 			         constraint::ErrorAggregator aggregator {}; */

	/* 			         ((aggregator.set(args)), ...); */

	/* 			         emplace_constraints_errors( */
	/* 			             errors, */
	/* 			             aggregator, */
	/* 			             request_body */
	/* 			         ); */
	/* 		         }, */
	/* 		         columns */
	/* 		     )), */
	/* 		     ...); */
	/* 	    }, */
	/* 	    T::columns */
	/* 	); */

	/* 	return errors; */
	/* } */

	/* template<typename T> */
	/* requires constraint::has_columns<T> bool */
	/* check_constraints( */
	/*     std::optional<::http::Response>& response, */
	/*     const nlohmann::json& request_body */
	/* ) */
	/* { */
	/* 	const std::vector<std::string> errors = */
	/* 	    get_constraint_errors<T>(request_body); */

	/* 	if (errors.size() > 0) */
	/* 	{ */
	/* 		response = */
	/* 		    std::make_optional(::http::Response {} */
	/* 		                           .code(::http::Code::BAD_REQUEST) */
	/* 		                           .content(create_errors_json(errors))); */
	/* 	} */

	/* 	return !response.has_value(); */
	/* } */

} // namespace util::json

#endif // UTIL_JSON_H_
