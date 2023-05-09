#include "routes/index.hpp"
#include "http/forward.hpp"
#include "http/parsor.hpp"
#include "http/query_parser.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "local/route_map.hpp"
#include "nlohmann/json.hpp"
#include "server/server.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/json_string_result.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "util/http.hpp"
#include "util/json.hpp"
#include <cctype>
#include <charconv>
#include <cmath>
#include <optional>
#include <sstream>
#include <string_view>
#include <tuple>

using R = http::Response;

http::Response get_foods(http::Request& request, RouteMap::BodyGetter&)
{
	std::optional<http::Response> response;

	auto& srv = server::Server::instance();
	sqlw::Statement stmt {srv.db_connection};

	http::Parsor parsor {request.query};
	auto query = (http::QueryParser {}).parse(parsor);

	const auto limit = query.contains("limit") ? query["limit"].value : "20";
	const auto page = query.contains("page") ? query["page"].value : "1";

	auto int_limit = 10;
	auto int_page = 1;
	std::from_chars(page.data(), page.data() + page.size(), int_page);
	std::from_chars(limit.data(), limit.data() + limit.size(), int_limit);

	// @todo: check from_chars

	const auto offset = (int_page - 1) * int_limit;

	size_t i = 0;
	int id = -1;

	for (auto it = request.target.crbegin(); it != request.target.crend();
	     ++it)
	{
		if (std::isdigit(*it))
		{
			id = (*it - '0') * std::pow(10, i);
			i++;
		}

		if (*it == '/')
		{
			break;
		}
	}

	std::stringstream ss;

	if (id != -1)
	{
		stmt.prepare("SELECT id,title,calories,proteins,carbohydrates,fats"
		             " FROM food WHERE id = ?")
		    .bind(1, std::to_string(id), sqlw::Type::SQL_INT);

		auto jsr = stmt.operator()<sqlw::JsonStringResult>();

		ss << "{\"data\":" << jsr.get_object_result() << "}";
	}
	else
	{
		stmt.prepare("SELECT id,title,calories,proteins,carbohydrates,fats"
		             " FROM food LIMIT ? OFFSET ?")
		    .bind(1, limit, sqlw::Type::SQL_INT)
		    .bind(2, std::to_string(offset), sqlw::Type::SQL_INT);

		auto jsr = stmt.operator()<sqlw::JsonStringResult>();

		ss << "{\"data\":" << jsr.get_array_result() << "}";
	}

	response.emplace(http::Response {}.content(ss.str()));

	!response.has_value() && util::http::fallback(response);

	return response.value();
};

http::Response
    post_foods(http::Request& request, RouteMap::BodyGetter& get_body)
{
	using njson = nlohmann::json;
	njson body = njson::parse(get_body());

	std::optional<http::Response> response;

	/* 	check_constraints<Food>(response, body) */
	/* 	    && transact( */
	/* 	        response, */
	/* 	        concat_queries<2>( */
	/* 	            {generate_insert_clause<Food>(), */
	/* 	             generate_select_last_insert(Food::table_name)} */
	/* 	        ), */
	/* 	        [&](sqlw::Statement& stmt) */
	/* 	        { */
	/* 		        bind_values<Food>(stmt, body); */
	/* 	        }, */
	/* 	        [](sqlw::JsonStringResult& json) */
	/* 	        { */
	/* 		        return json.get_object_result(); */
	/* 	        } */
	/* 	    ) */

	auto& srv = server::Server::instance();
	sqlw::Statement stmt {srv.db_connection};

	using is_required_t = bool;
	using column_name_t = std::string_view;

	constexpr std::array<std::tuple<column_name_t, sqlw::Type, is_required_t>, 5>
	    columns {
	        {{"title", sqlw::Type::SQL_TEXT, true},
	         {"calories", sqlw::Type::SQL_DOUBLE, false},
	         {"proteins", sqlw::Type::SQL_DOUBLE, false},
	         {"carbohydrates", sqlw::Type::SQL_DOUBLE, false},
	         {"fats", sqlw::Type::SQL_DOUBLE, false}}
    };

	stmt.prepare("INSERT INTO food (title,calories,proteins,carbohydrates,fats)"
	             " VALUES (?,?,?,?,?)");

	std::vector<std::string> errors;
	size_t arg_idx = 0;
	for (const auto& column : columns)
	{
		arg_idx++;

		std::string_view val;

		if (body.find(std::get<column_name_t>(column)) == body.end()
		    || body[std::get<column_name_t>(column)].is_null())
		{
			if (std::get<is_required_t>(column))
			{
				errors.push_back(
				    std::string {std::get<column_name_t>(column)}
				    + " is required"
				);
			}
			else
			{
				switch (std::get<sqlw::Type>(column))
				{
					case sqlw::Type::SQL_DOUBLE:
						val = "0.0";
						break;
					case sqlw::Type::SQL_INT:
						val = "0";
						break;
					case sqlw::Type::SQL_TEXT:
					case sqlw::Type::SQL_BLOB:
					case sqlw::Type::SQL_NULL:
						val = "NULL";
						break;
				}
			}
		}
		else
		{
			switch (std::get<sqlw::Type>(column))
			{
				case sqlw::Type::SQL_DOUBLE:
				case sqlw::Type::SQL_INT:
					val = body[std::get<column_name_t>(column)].dump();
					break;
				case sqlw::Type::SQL_TEXT:
				case sqlw::Type::SQL_BLOB:
					val = util::json::remove_quotes(
					    body[std::get<column_name_t>(column)].dump()
					);
					break;
				case sqlw::Type::SQL_NULL:
					val = "null";
					break;
			}
		}

		if (errors.empty())
		{
			stmt.bind(arg_idx, val, std::get<sqlw::Type>(column));
		}
	}

	if (!errors.empty())
	{
		response.emplace(http::Response {}
		                     .code(::http::Code::BAD_REQUEST)
		                     .content_type(::http::ContentType::APP_JSON)
		                     .content(util::json::create_errors_json(errors)));
	}

	stmt.exec();

	auto json = stmt.operator()<sqlw::JsonStringResult>(
	    "SELECT id,title,calories,proteins,carbohydrates,fats FROM food WHERE id = last_insert_rowid()"
	);

	response.emplace(
	    http::Response {}
	        .content_type(http::ContentType::APP_JSON)
	        .content("{\"data\":" + json.get_object_result() + "}")
	);

	!response.has_value() && util::http::fallback(response);

	return response.value();
};

RouteMap::RouteMap()
{
	add(http::Method::GET, "/api/foods", get_foods);
	add(http::Method::POST, "/api/foods", post_foods);

	/* add(http::Method::POST, "/api/foods", post_foods); */

	/* add(http::Method::GET, */
	/*     "/api/recipes", */
	/*     [](http::Request& request) */
	/*     { */
	/* 	    std::optional<http::Response> response; */

	/* 	    exec_query( */
	/* 	        response, */
	/* 	        R"(SELECT */
	/* 				f.*, */
	/* 				( */
	/* 					SELECT */
	/* 						json_group_array(i.id) */
	/* 					FROM food i */
	/* 					INNER JOIN food_composition fc */
	/* 						ON fc.particular_id = i.id AND fc.composite_id =
	 * f.id
	 */
	/* 					GROUP BY fc.composite_id */
	/* 				) ingredient_ids */
	/* 			FROM food f */
	/* 			WHERE ingredient_ids IS NOT NULL */
	/* 			LIMIT ?)", */
	/* 	        [](sqlw::Statement& stmt) */
	/* 	        { */
	/* 		        stmt.bind(1, "10", sqlw::Type::SQL_INT); */
	/* 	        }, */
	/* 	        [](sqlw::JsonStringResult& result) */
	/* 	        { */
	/* 		        return result.get_array_result(); */
	/* 	        } */
	/* 	    ) && fallback(response); */

	/* 	    return response.value(); */
	/*     }); */

	/* add(http::Method::GET, */
	/*     "/", */
	/*     [](http::Request&) */
	/*     { */
	/* 	    return R {}.content( */
	/* 	        "<!DOCTYPE html>" */
	/* 	        "<html>" */
	/* 	        "<body>" */
	/* 	        "INDEX PAGE" */
	/* 	        "<img src=\"/The_army_of_titanium_dioxide_nanotubes(1).jpg\"/>"
	 */
	/* 	        "</body>" */
	/* 	        "</html>" */
	/* 	    ); */
	/*     }); */
}
