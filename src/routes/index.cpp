#include "routes/index.hpp"
#include "api/api.hpp"
#include "http/forward.hpp"
#include "http/parser.hpp"
#include "http/parsor.hpp"
#include "http/query_parser.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "local/route_map.hpp"
#include "nlohmann/json.hpp"
#include "server/server.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/json_string_result.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "util/base64.hpp"
#include "util/http.hpp"
#include "util/json.hpp"
#include <cctype>
#include <charconv>
#include <chrono>
#include <cmath>
#include <concepts>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>

using R = http::Response;
using njson = nlohmann::json;

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

	response.emplace(http::Response {}
	                     .content_type(http::ContentType::APP_JSON)
	                     .content(ss.str()));

	!response.has_value() && util::http::fallback(response);

	return response.value();
};

class JsonValueExtractor
{
public:
	JsonValueExtractor(std::string_view json)
	{
		try
		{
			m_json = njson::parse(json);
		}
		catch (const njson::exception excp)
		{
			m_errors.emplace_back("Erroneous reqeuest body");
		}
	}

	auto exists(api::column_name_t col_name) -> bool
	{
		return m_json.find(col_name) != m_json.end();
	}

	auto is_null(api::column_name_t col_name) -> bool
	{
		return m_json[col_name].is_null();
	}

	auto value(api::column_name_t col_name) -> std::string
	{
		return util::json::remove_quotes(m_json[col_name].dump());
	}

	auto errors() -> std::vector<std::string>&
	{
		return m_errors;
	}

private:
	njson m_json;
	std::vector<std::string> m_errors;
};

static_assert(api::is_value_extractor<JsonValueExtractor>);

struct ApiRequestState
{
	http::Request& request;
	RouteMap::BodyGetter& get_body;
	std::string username;
	std::optional<http::Response> response;
};

bool try_post_food(ApiRequestState& state)
{
	auto& srv = server::Server::instance();
	sqlw::Statement stmt {srv.db_connection};

	auto r = api::create_food<JsonValueExtractor, sqlw::JsonStringResult>(
	    state.get_body(),
	    state.username,
	    srv.db_connection
	);

	if (std::get<1>(r).size() > 0)
	{
		state.response.emplace(
		    http::Response {}
		        .code(::http::Code::BAD_REQUEST)
		        .content_type(::http::ContentType::APP_JSON)
		        .content(util::json::create_errors_json(std::get<1>(r)))
		);
	}
	else
	{
		state.response.emplace(
		    http::Response {}
		        .content_type(http::ContentType::APP_JSON)
		        .content(
		            "{\"data\":" + std::get<0>(r).get_object_result() + "}"
		        )
		);
	}

	return !state.response.has_value();
}

bool emplace_basic_auth_request(std::optional<http::Response>& response)
{
	response.emplace(
	    http::Response {}
	        .code(http::Code::UNAUTHORIZED)
	        .headers(
	            "WWW-Authenticate: Basic realm=\"API for authorized users\""
	        )
	);

	return false;
}

bool authenticate(ApiRequestState& state)
{
	auto& srv = server::Server::instance();
	sqlw::Statement stmt {srv.db_connection};

	http::Parsor p1 {state.request.raw};
	const auto headers = http::Parser {}.parse(
	    p1,
	    http::parser::Context<1, 1> {
	        .names = {"Authorization"},
	        .value_delimiters = {"\r\n"},
	        .declaration_delimiter = ":",
	        .start_str = "",
	    }
	);

	if (headers[0].length() == 0)
	{
		return emplace_basic_auth_request(state.response);
	}

	auto pos = headers[0].find("Basic ");

	if (pos == std::string_view::npos)
	{
		return emplace_basic_auth_request(state.response);
	}

	const auto basic = headers[0].substr(pos + 6);

	const auto login = decode_base64(basic);

	if (login[login.length() - 1] != ':')
	{
		return emplace_basic_auth_request(state.response);
	}

	// @todo: cache this query.
	std::string username;
	stmt.prepare("SELECT name FROM \"user\""
	             " WHERE name = ?")
	    .bind(1, login.substr(0, login.length() - 1), sqlw::Type::SQL_TEXT);
	stmt(
	    [&username](sqlw::Statement::ExecArgs args)
	    {
		    username = args.column_value;
	    }
	);

	if (username.empty())
	{
		return emplace_basic_auth_request(state.response);
	}

	state.username = std::move(username);

	return true;
}

http::Response
    post_foods(http::Request& request, RouteMap::BodyGetter& get_body)
{
	ApiRequestState state {request, get_body, {}, {}};

	authenticate(state) && try_post_food(state)
	    && util::http::fallback(state.response);

	return state.response.value();
};

RouteMap::RouteMap()
{
	add(http::Method::GET, "/api/foods", get_foods);
	add(http::Method::POST, "/api/foods", post_foods);

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
	    [](auto, auto)
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
