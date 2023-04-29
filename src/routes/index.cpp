#include "routes/index.hpp"
#include "http/forward.hpp"
#include "http/parsor.hpp"
#include "http/query_parser.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "local/route_map.hpp"
#include "server/server.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/json_string_result.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "util/http.hpp"
#include <charconv>
#include <optional>

using R = http::Response;

auto get_foods = [](http::Request& request)
{
	std::optional<http::Response> response;

	auto& srv = server::Server::instance();
	sqlw::Statement stmt {srv.db_connection};

	http::Parsor parsor {request.query};
	auto query = (http::QueryParser{}).parse(parsor);

	const auto limit = query.contains("limit") ? query["limit"].value : "20";
	const auto page = query.contains("page") ? query["page"].value : "1";

	auto int_limit = 10;
	auto int_page = 1;
	std::from_chars(page.data(), page.data() + page.size(), int_page);
	std::from_chars(limit.data(), limit.data() + limit.size(), int_limit);

	const auto offset = (int_page - 1) * int_limit;

	std::cout << request.query << '\n';
	stmt
		.prepare("SELECT * FROM food LIMIT ? OFFSET ?")
	    .bind(1, limit, sqlw::Type::SQL_INT)
	    .bind(2, std::to_string(offset), sqlw::Type::SQL_INT);
	auto jsr = stmt.operator()<sqlw::JsonStringResult>();

	response.emplace(http::Response {}.content(jsr.get_array_result()));

	/* exec_query( */
	/*     response, */
	/*     "SELECT f.* FROM food WHERE LIMIT ?", */
	/*     [](sqlw::Statement& stmt) */
	/*     { */
	/* 	    stmt.bind(1, "10", sqlw::Type::SQL_INT); */
	/*     }, */
	/*     [](sqlw::JsonStringResult& result) */
	/*     { */
	/* 	    return result.get_array_result(); */
	/*     } */
	/* ) && */
	!response.has_value() && util::http::fallback(response);

	return response.value();
};

/* auto post_foods = [](http::Request& request) */
/* { */
/* 	using json = nlohmann::json; */
/* 	json body = json::parse(request.body); */

/* 	std::optional<http::Response> response; */

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
/* 	    && fallback(response); */

/* 	return response.value(); */
/* }; */

RouteMap::RouteMap()
{
	add(http::Method::GET, "/api/foods", get_foods);

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

	add(http::Method::GET,
	    "/",
	    [](http::Request&)
	    {
		    return R {}.content(
		        "<!DOCTYPE html>"
		        "<html>"
		        "<body>"
		        "INDEX PAGE"
		        "<img src=\"/The_army_of_titanium_dioxide_nanotubes(1).jpg\"/>"
		        "</body>"
		        "</html>"
		    );
	    });
}
