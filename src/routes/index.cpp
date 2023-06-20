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
#include "sqlw/concepts.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/json_string_result.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "util/base64.hpp"
#include "util/http.hpp"
#include "util/json.hpp"
#include <algorithm>
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

bool are_ids_valid(std::string_view ids)
{
	for (const char c : ids)
	{
		if (!(c == ',' || std::isdigit(c)))
		{
			return false;
		}
	}

	return true;
}

constexpr std::string_view select_food()
{
	return "SELECT id,title,calories,proteins,carbohydrates,fats FROM food f";
}

constexpr std::string_view select_cooking_action()
{
	return "SELECT id,title,description FROM cooking_action ca";
}

struct GetParams
{
	GetParams(
	    std::string_view a_target,
	    std::string_view a_limit,
	    std::string_view a_page,
	    std::string_view a_ids
	)
	{
		this->ids = a_ids;
		this->limit = a_limit;

		int _limit;

		// @todo: check from_chars
		std::from_chars(
		    a_page.data(),
		    a_page.data() + a_page.size(),
		    this->page
		);
		std::from_chars(
		    a_limit.data(),
		    a_limit.data() + a_limit.size(),
		    _limit
		);

		this->offset = (this->page - 1) * _limit;

		size_t i = 0;
		for (auto it = a_target.crbegin(); it != a_target.crend(); ++it)
		{
			if (std::isdigit(*it))
			{
				this->id = (*it - '0') * std::pow(10, i);
				i++;
			}

			if (*it == '/')
			{
				break;
			}
		}
	}

	int id {-1};
	std::string_view limit {"10"};
	int page {1};
	int offset {0};
	std::string_view ids {""};
};

enum class QueryType
{
	ID,
	IDS,
	OFFSET,
};

template<typename T>
requires sqlw::can_be_used_by_statement<T>
std::tuple<QueryType, T> exec_get_params_query(
    const GetParams& gp,
    sqlw::Connection* con,
    std::string_view sql,
    std::string_view table_specifier,
    std::string_view post_where_sql = ""
)
{
	sqlw::Statement stmt {con};

	std::stringstream ss;
	std::stringstream sql_ss;
	sql_ss << sql;

	if (gp.id != -1)
	{
		sql_ss << " WHERE " << table_specifier << ".id = ? " << post_where_sql;
		stmt.prepare(sql_ss.str())
		    .bind(1, std::to_string(gp.id), sqlw::Type::SQL_INT);

		return {QueryType::ID, stmt.operator()<T>()};
	}
	else if (gp.ids.length() > 0 && are_ids_valid(gp.ids))
	{
		sql_ss << " WHERE " << table_specifier << ".id IN (" << gp.ids << ") "
		       << post_where_sql;

		stmt.prepare(sql_ss.str());

		return {QueryType::IDS, stmt.operator()<T>()};
	}

	sql_ss << " " << post_where_sql << " LIMIT ? OFFSET ? ";
	stmt.prepare(sql_ss.str())
	    .bind(1, gp.limit, sqlw::Type::SQL_INT)
	    .bind(2, std::to_string(gp.offset), sqlw::Type::SQL_INT);

	return {QueryType::OFFSET, stmt.operator()<T>()};
};

http::Response generic_get(
    http::Request& request,
    std::string_view select_sql,
    std::string_view table_specifier,
    std::string_view post_where_sql = ""
)
{
	std::optional<http::Response> response;

	auto& srv = server::Server::instance();

	http::Parsor parsor {request.query};
	auto query = (http::QueryParser {}).parse(parsor);

	const auto gp = GetParams {
	    request.target,
	    query.contains("limit") ? query["limit"].value : "10",
	    query.contains("page") ? query["page"].value : "1",
	    query.contains("ids") ? query["ids"].value : ""};

	auto r = exec_get_params_query<sqlw::JsonStringResult>(
	    gp,
	    srv.db_connection,
	    select_sql,
	    table_specifier,
	    post_where_sql
	);

	std::stringstream ss;

	if (std::get<QueryType>(r) == QueryType::ID)
	{
		ss << "{\"data\":"
		   << std::get<sqlw::JsonStringResult>(r).get_object_result() << "}";
	}
	else
	{
		ss << "{\"data\":"
		   << std::get<sqlw::JsonStringResult>(r).get_array_result() << "}";
	}

	response.emplace(http::Response {}
	                     .content_type(http::ContentType::APP_JSON)
	                     .content(ss.str()));

	!response.has_value() && util::http::fallback(response);

	return response.value();
};

http::Response get_foods(http::Request& request, RouteMap::BodyGetter&)
{
	return generic_get(request, select_food(), "f");
};

http::Response get_recipes(http::Request& request, RouteMap::BodyGetter&)
{
	return generic_get(
	    request,
	    "SELECT "
	    "f.id food, "
	    "'[' || GROUP_CONCAT(rs.priority, ',') || ']' steps, "
	    "'[' || GROUP_CONCAT(rs.cooking_action_id, ',') || ']' actions, "
	    "'[' || GROUP_CONCAT(rsf.food_id, ',') || ']' foods "
	    "FROM food f "
	    "INNER JOIN recipe_step rs "
	    "ON rs.recipe_id = f.id "
	    "INNER JOIN recipe_step_food rsf "
	    "ON rsf.recipe_step_id = rs.id ",
	    "f",
	    "GROUP BY f.id "
	    "ORDER BY rs.priority "
	);
};

http::Response
    get_cooking_actions(http::Request& request, RouteMap::BodyGetter&)
{
	return generic_get(
	    request,
	    select_cooking_action(),
	    "ca",
	    "ORDER BY ca.id ASC"
	);
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

bool try_post_cooking_action(ApiRequestState& state)
{
	auto& srv = server::Server::instance();
	sqlw::Statement stmt {srv.db_connection};

	auto r =
	    api::create_cooking_action<JsonValueExtractor, sqlw::JsonStringResult>(
	        state.get_body(),
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
	const auto headers = (http::Parser {})
	                         .parse(
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

http::Response
    post_cooking_actions(http::Request& request, RouteMap::BodyGetter& get_body)
{
	ApiRequestState state {request, get_body, {}, {}};

	authenticate(state) && try_post_cooking_action(state)
	    && util::http::fallback(state.response);

	return state.response.value();
};

auto check_numeric_value_operator(const std::string_view& value)
{
	if (value.length() > 2
	    && (('<' == value[0] && '=' == value[1])
	        || ('>' == value[0] && '=' == value[1])))
	{
		return 2;
	}
	else if (value.length() > 1 && ('<' == value[0] || '>' == value[0]))
	{
		return 1;
	}

	return 0;
}

void construct_filter_clause(
    std::stringstream& sql_ss,
    const std::tuple<std::string_view, sqlw::Type>& field,
    const std::string_view& value
)
{
	switch (std::get<sqlw::Type>(field))
	{
		case sqlw::Type::SQL_DOUBLE:
			{
				const auto check = check_numeric_value_operator(value);

				sql_ss << std::get<std::string_view>(field) << ' '
				       << value.substr(0, check) << " ?";
			}
			break;
		default:
			sql_ss << std::get<std::string_view>(field)
			       << " LIKE '%' || ? || '%' ";
	}
}

template<size_t Size>
http::Response filter(
    const std::array<std::tuple<std::string_view, sqlw::Type>, Size>&
        allowed_fields,
    std::string_view select_sql,
    http::Request& request,
    sqlw::Connection* db_connection
)
{
	sqlw::Statement stmt {db_connection};
	http::Parsor parsor {request.query};
	auto query = (http::QueryParser {}).parse(parsor);

	std::stringstream sql_ss;
	sql_ss << select_sql << " WHERE ";

	int i = 0;
	for (const auto& field : allowed_fields)
	{
		if (!query.contains(std::get<std::string_view>(field)))
		{
			continue;
		}

		if (i++ > 0)
		{
			sql_ss << " AND ";
		}

		const auto& q_value = query.at(std::get<std::string_view>(field));

		if (q_value.values.size() > 0)
		{
			sql_ss << " ( ";

			size_t j = 0;
			for (const auto& value : q_value.values)
			{
				if (j++ > 0)
				{
					sql_ss << " OR ";
				}

				construct_filter_clause(sql_ss, field, value);
			}

			sql_ss << " ) ";
		}
		else
		{
			construct_filter_clause(sql_ss, field, q_value.value);
		}
	}

	stmt.prepare(sql_ss.str());

	i = 0;
	for (const auto& field : allowed_fields)
	{
		if (!query.contains(std::get<std::string_view>(field)))
		{
			continue;
		}

		const auto& q_value = query.at(std::get<std::string_view>(field));

		if (q_value.values.size() > 0)
		{
			for (const auto& value : q_value.values)
			{
				const auto check = check_numeric_value_operator(value);
				// @todo: add try incase when value is not convertible to
				// number
				stmt.bind(
				    ++i,
				    value.substr(check),
				    std::get<sqlw::Type>(field)
				);
			}
		}
		else
		{
			const auto check = check_numeric_value_operator(q_value.value);
			// @todo: add try incase when value is not convertible to number
			stmt.bind(
			    ++i,
			    q_value.value.substr(check),
			    std::get<sqlw::Type>(field)
			);
		}
	}

	auto json = stmt.operator()<sqlw::JsonStringResult>();

	std::stringstream ss;
	ss << "{\"data\":" << json.get_array_result() << "}";

	return http::Response {}
	    .content_type(http::ContentType::APP_JSON)
	    .content(ss.str());
}

http::Response filter_foods(http::Request& request, RouteMap::BodyGetter&)
{
	auto& srv = server::Server::instance();

	constexpr std::array<std::tuple<std::string_view, sqlw::Type>, 2>
	    allowed_fields {
	        {
             {"title", sqlw::Type::SQL_TEXT},
             {"calories", sqlw::Type::SQL_DOUBLE},
	         }
    };

	return filter(allowed_fields, select_food(), request, srv.db_connection);
};

http::Response
    filter_cooking_actions(http::Request& request, RouteMap::BodyGetter&)
{
	auto& srv = server::Server::instance();

	constexpr std::array<std::tuple<std::string_view, sqlw::Type>, 1>
	    allowed_fields {
	        {
             {"title", sqlw::Type::SQL_TEXT},
	         }
    };

	return filter(
	    allowed_fields,
	    select_cooking_action(),
	    request,
	    srv.db_connection
	);
};

RouteMap::RouteMap()
{
	add(http::Method::GET, "/api/foods", get_foods);
	add(http::Method::GET, "/api/foods/filter", filter_foods);
	add(http::Method::POST, "/api/foods", post_foods);

	add(http::Method::GET, "/api/recipes", get_recipes);

	add(http::Method::GET, "/api/cooking_actions", get_cooking_actions);
	add(http::Method::GET,
	    "/api/cooking_actions/filter",
	    filter_cooking_actions);
	add(http::Method::POST, "/api/cooking_actions", post_cooking_actions);

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
