#include "db/db.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

static void log_migration(
    sqlw::Connection& con,
    const std::filesystem::directory_entry& entry
)
{
	std::stringstream insert;
	const auto timestamp = std::time(nullptr);

	insert << "INSERT INTO migration (filename, executed_at_stamp) "
	       << "VALUES ('" << entry.path().filename().string() << "', '"
	       << timestamp << "')";

	sqlw::Statement {&con}(insert.view());
}

static bool check_meta_table_exists(sqlw::Connection& con)
{
	// get list of executed migrations
	bool result = false;

	sqlw::Statement {&con}(
	    "SELECT name "
	    "FROM sqlite_master "
	    "WHERE type='table' AND name='migration'",
	    [&result](sqlw::Statement::ExecArgs _)
	    {
		    result = true;
	    }
	);

	return result;
}

static std::vector<std::string> find_executed_migrations(sqlw::Connection& con)
{
	std::vector<std::string> result;
	const auto meta_table_exists = check_meta_table_exists(con);

	if (!meta_table_exists || !sqlw::status::is_ok(con.status()))
	{
		return result;
	}

	sqlw::Statement {&con}(
	    "SELECT filename FROM migration",
	    [&result](sqlw::Statement::ExecArgs args)
	    {
		    result.push_back(std::string {args.column_value});
	    }
	);

	return result;
}

static bool check_if_migrated_already(
    const std::filesystem::directory_entry& entry,
    const std::vector<std::string>& executed_migrations
)
{
	const auto it = std::find_if(
	    executed_migrations.begin(),
	    executed_migrations.end(),
	    [&entry](const std::string& name)
	    {
		    return name == entry.path().filename();
	    }
	);

	return it != executed_migrations.end();
}

static void execute_migration(
    sqlw::Connection& con,
    const std::filesystem::directory_entry& entry
)
{
	const auto fstream = std::ifstream {entry.path()};
	auto sstream = std::stringstream {};
	sstream << fstream.rdbuf();

	sqlw::Statement {&con}(sstream.view());
}

static void print_sql_error(const sqlw::Connection& con)
{
	std::cout << '(' << static_cast<int>(con.status()) << ") "
	          << sqlw::status::verbose(con.status()) << '\n';
}

/**
 * @todo
 * 	- Add return codes;
 * 	- Add tests for return codes.
 */
int db::migration::migrate(
    std::filesystem::path database_path,
    std::filesystem::path migrations_dir
)
{
	if (!std::filesystem::exists(database_path.parent_path())
	    || !std::filesystem::exists(migrations_dir))
	{
		return 10;
	}

	sqlw::Connection con {database_path.string()};
	const auto executed_migrations = find_executed_migrations(con);

	std::cout << "A total of " << executed_migrations.size()
	          << " already executed migrations found\n";

	if (!sqlw::status::is_ok(con.status()))
	{
		print_sql_error(con);
		return 1;
	}

	sqlw::Statement stmt {&con};

	const auto path = std::filesystem::path {migrations_dir};

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (check_if_migrated_already(entry, executed_migrations))
		{
			continue;
		}

		stmt("BEGIN EXCLUSIVE TRANSACTION;"
		     "SAVEPOINT actual_migration_sp;");

		std::cout << "migrating " << entry.path() << "\n";
		execute_migration(con, entry);

		if (!sqlw::status::is_ok(con.status()))
		{
			print_sql_error(con);

			stmt("ROLLBACK TO actual_migration_sp;"
			     "ROLLBACK;");

			return 1;
		}

		stmt("RELEASE actual_migration_sp");

		stmt("SAVEPOINT migration_log_sp");

		std::cout << "logging migration " << entry.path().filename().string()
		          << "\n";
		log_migration(con, entry);

		if (!sqlw::status::is_ok(con.status()))
		{
			print_sql_error(con);

			stmt("ROLLBACK TO migration_log_sp;"
			     "ROLLBACK;");

			return 1;
		}

		stmt("RELEASE migration_log_sp;"
		     "COMMIT;");
	}

	return 0;
}

/* std::string */
/*     db::generate_select_last_insert(const std::string_view table_name) */
/* { */
/* 	std::stringstream ss; */
/* 	ss << "SELECT e0.* FROM " << table_name << " e0 " */
/* 	   << "WHERE e0.id = last_insert_rowid()"; */
/* 	; */

/* 	return ss.str(); */
/* } */

/* sqlw::Connection */
/*     db::init_db_and_write_status(std::optional<::http::Response>& response
 */
/*     ) */
/* { */
/* 	auto con = db::init_db_connection(); */

/* 	if (!sqlw::status::is_ok(con.status())) */
/* 	{ */
/* 		response.emplace(http::generate_db_error_response(con)); */
/* 	} */

/* 	return con; */
/* } */

/* bool db::exec_query( */
/*     std::optional<::http::Response>& response, */
/*     const std::string_view sql, */
/*     std::function<void(sqlw::Statement&)> binder, */
/*     std::function<std::string(sqlw::JsonStringResult&)> extractor */
/* ) */
/* { */
/* 	auto con = db::init_db_and_write_status(response); */

/* 	if (response.has_value()) */
/* 	{ */
/* 		return false; */
/* 	} */

/* 	sqlw::Statement stmt {&con}; */

/* 	if (binder) */
/* 	{ */
/* 		binder(stmt); */
/* 	} */

/* 	if (!sqlw::status::is_ok(stmt.status())) */
/* 	{ */
/* 		response.emplace(http::generate_stmt_error_response(stmt)); */

/* 		return false; */
/* 	} */

/* 	if (extractor) */
/* 	{ */
/* 		auto json_result = stmt.operator()<sqlw::JsonStringResult>(sql); */

/* 		response.emplace(::http::Response {} */
/* 		                     .content(extractor(json_result)) */
/* 		                     .content_type(::http::ContentType::APP_JSON)); */
/* 	} */

/* 	return !response.has_value(); */
/* } */

/* bool db::transact( */
/*     std::optional<::http::Response>& response, */
/*     const std::string sql, */
/*     std::function<void(sqlw::Statement& stmt)> binder, */
/*     std::function<std::string(sqlw::JsonStringResult&)> emplacer */
/* ) */
/* { */
/* 	auto con = db::init_db_and_write_status(response); */

/* 	if (response.has_value()) */
/* 	{ */
/* 		return true; */
/* 	} */

/* 	sqlw::Statement stmt {&con}; */

/* 	stmt("BEGIN DEFERRED TRANSACTION"); */

/* 	binder(stmt); */

/* 	auto r = stmt.operator()<sqlw::JsonStringResult>(sql); */

/* 	if (!sqlw::status::is_ok(stmt.status())) */
/* 	{ */
/* 		response.emplace(http::generate_stmt_error_response(stmt)); */

/* 		stmt("ROLLBACK"); */
/* 	} */
/* 	else */
/* 	{ */
/* 		stmt("COMMIT"); */
/* 	} */

/* 	if (r.has_result()) */
/* 	{ */
/* 		response->content(emplacer(r)); */
/* 	} */

/* 	return !response.has_value(); */
/* } */
