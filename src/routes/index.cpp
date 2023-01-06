#include "routes/index.hpp"
#include "config.h"
#include "http/request.hpp"
#include "sql/forward.hpp"
#include "sql/statement.hpp"
#include "sql/status.hpp"
#include "tables/definitions.hpp"
#include "tables/utils.hpp"
#include "utils.hpp"
#include "local/route_map.hpp"
#include "sql/sqlite3.hpp"
#include "sql/json_string_result.hpp"
#include "nlohmann/json.hpp"
#include <sstream>
#include <string_view>
#include <vector>

using R = http::Response;

static std::string create_errors_json(const std::vector<std::string>& errors)
{
	std::stringstream ss;

	ss << "{\"errors\": [";

	for (const auto& error : errors)
	{
		ss << '"' << error << '"';
	}

	ss << "]}";

	return ss.str();
}

static http::Response generate_db_error_response()
{
	return R {}
		.code(http::Code::INTERNAL_SERVER_ERROR)
		.content(create_errors_json({"Что-то нет так с БД."}))
		.content_type(http::ContentType::APP_JSON)
	;
}

static http::Response generate_stmt_error_response(const sql::Statement& stmt)
{
	return R {}
		.code(http::Code::BAD_REQUEST)
		.content(create_errors_json({sql::status::verbose(stmt)}))
		.content_type(http::ContentType::APP_JSON)
	;
}

static sql::Sqlite3 get_db()
{
	return {"culinary.db"};
}

static http::Response generic_get(
	const tables::Definition* definition,
	http::Request& request
)
{
	auto db = get_db();

	// check if target is refering to cpecific entity by its id.

	/* CLOG(request.target); */
	/* CLOG(request.query); */
	std::stringstream ss;
	ss << "SELECT e0.* FROM " << definition->table_name << " e0";

	auto json = db.exec<sql::JsonStringResult>(ss.str());

	return R {}
		.content(json.get_array_result())
		.content_type(http::ContentType::APP_JSON)
	;
}

static http::Response generic_post(
	const tables::Definition* definition,
	http::Request& request
)
{
	using json = nlohmann::json;

	auto db = get_db();

	json j = json::parse(request.body);

	sql::Statement stmt {
		db,
		tables::generate_insert_clause(*definition)
	};
	
	if (!sql::status::is_ok(db))
	{
		return generate_db_error_response();
	}

	const auto errors = tables::get_constraints_errors(*definition, j);

	if (errors.size() > 0)
	{
		return R {}
			.code(http::Code::BAD_REQUEST)
			.content(create_errors_json(errors))
		;
	}

	tables::bind_values(*definition, stmt, j);
	stmt.exec();

	if (!sql::status::is_ok(stmt))
	{
		return generate_stmt_error_response(stmt);
	}

	std::stringstream ss;
	ss << "SELECT e0.* FROM " << definition->table_name << " e0 "
		<< "WHERE e0.id = last_insert_rowid()";
	;
	sql::Statement result_stmt {db, ss.str()};
	auto result = result_stmt.exec<sql::JsonStringResult>();

	if (!sql::status::is_ok(result_stmt))
	{
		return generate_stmt_error_response(stmt);
	}

	return R {}
		.content(result.get_object_result())
		.content_type(http::ContentType::APP_JSON)
	;
}

struct EndpointDefinition
{
	const std::string_view table_name;
	const std::string_view endpoint_name;
};

RouteMap::RouteMap()
{

	/* const std::vector<EndpointDefinition> endpoint_definitions = { */
	/* 	{ "food", "foods"}, */
	/* }; */

	for (const auto& definition : tables::definitions())
	{
		/* const auto endpoint_definition = std::ranges::find_if( */
		/* 	endpoint_definitions, */
		/* 	[&definition](const EndpointDefinition& r){return r.table_name == definition.table_name;} */
		/* ); */

		/* if (endpoint_definition == endpoint_definitions.end()) */
		/* { */
		/* 	continue; */
		/* } */

		const auto* definition_ptr = &definition;
		add(
			http::Method::GET,
			std::string{"/api/"} + std::string{definition.table_name},
			[definition_ptr](http::Request& request)
			{
				return generic_get(definition_ptr, request);
			}
		);
		add(
			http::Method::POST,
			std::string{"/api/"} + std::string{definition.table_name},
			[definition_ptr](http::Request& request)
			{
				return generic_post(definition_ptr, request);
			}
		);
	}

	add(
		http::Method::GET,
		"/api/recipes",
		[](http::Request& request)
		{
			auto db = get_db();

			if (!sql::status::is_ok(db))
			{
				return generate_db_error_response();
			}

			sql::Statement stmt {
				db,
				R"(SELECT
					f.*,
					(
						SELECT
							json_group_array(i.id)
						FROM food i
						INNER JOIN food_composition fc
							ON fc.particular_id = i.id AND fc.composite_id = f.id
						GROUP BY fc.composite_id
					) ingredient_ids
				FROM food f
				WHERE ingredient_ids IS NOT NULL
				LIMIT ?)"
			};

			stmt.bind(1, "10", sql::Type::SQL_INT);

			auto result = stmt.exec<sql::JsonStringResult>();

			return R {}
				.content(result.get_array_result())
				.content_type(http::ContentType::APP_JSON)
			;
		}
	);

	add(
		http::Method::GET,
		"/",
		[](http::Request&){
			return R {}.content(
				"<!DOCTYPE html>"
				"<html>"
				"<body>"
				"INDEX PAGE"
				"<img src=\"/The_army_of_titanium_dioxide_nanotubes(1).jpg\"/>"
				"</body>"
				"</html>"
			);
		}
	);
}
