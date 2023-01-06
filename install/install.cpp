#include "sql/sqlite3.hpp"
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include "config.h"
#include "sql/status.hpp"

static int table_exists(
	void* obj,
	int argc,
	char** argv,
	char** col_name
)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		auto result = static_cast<bool*>(obj);
		*result = true;
		
		break;
	}

	return 0;
}

static int fill_executed_migrations_vector(
	void* obj,
	int argc,
	char** argv,
	char** col_name
)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		auto result = static_cast<std::vector<std::string>*>(obj);
		result->push_back(argv[i]);
	}

	return 0;
}

static void print_sql_error(const sql::Sqlite3& db)
{
	std::cout << "(" << sql::status::view(db) << ") " << db.message() << "\n";
}

static bool check_meta_table_exists(sql::Sqlite3& db)
{
	// get list of executed migrations
	bool result = false;

	db.exec(
		"SELECT name "
		"FROM sqlite_master "
		"WHERE type='table' AND name='migration'",
		table_exists,
		&result
	);

	return result;
}

static std::vector<std::string> get_executed_migrations(sql::Sqlite3& db)
{
	std::vector<std::string> result;
	const auto meta_table_exists = check_meta_table_exists(db);

	if (!meta_table_exists || !sql::status::is_ok(db))
	{
		return result;
	}

	db.exec(
		"SELECT filename FROM migration",
		fill_executed_migrations_vector,
		&result
	);

	return result;
}

static bool check_if_migrated_already(sql::Sqlite3& db, const std::filesystem::directory_entry& entry, const std::vector<std::string>& executed_migrations)
{
	const auto it = std::find_if(
		executed_migrations.begin(),
		executed_migrations.end(),
		[&entry] (const std::string& name) {
			return name == entry.path().filename();
		}
	);

	return it != executed_migrations.end();
}

static void execute_migration(sql::Sqlite3& db, const std::filesystem::directory_entry& entry)
{
	const auto fstream = std::ifstream{entry.path()};
	auto sstream = std::stringstream{};
	sstream << fstream.rdbuf();

	db.exec(sstream.str());
}

static void log_migration(sql::Sqlite3& db, const std::filesystem::directory_entry& entry)
{
	std::stringstream insert;
	const auto timestamp = std::time(nullptr);

	insert
		<< "INSERT INTO migration (filename, executed_at_stamp) "
		<< "VALUES ('" << entry.path().filename().string() << "', '" << timestamp <<"')"
	;

	db.exec(insert.str());
}

int main()
{
	sql::Sqlite3 db{"culinary.db"};
	const auto executed_migrations = get_executed_migrations(db);

	std::cout << "A total of " << executed_migrations.size() << " migrations found\n";

	if (!sql::status::is_ok(db))
	{
		print_sql_error(db);
		return 1;
	}

	const auto path = std::string{CMAKE_CURRENT_SOURCE_DIR} + "/migrations";

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (check_if_migrated_already(db, entry, executed_migrations))
		{
			continue;
		}

		db.exec(
			"BEGIN EXCLUSIVE TRANSACTION;"
			"SAVEPOINT actual_migration_sp;"
		);

		std::cout << "migrating " << entry.path() << "\n";
		execute_migration(db, entry);

		if (!sql::status::is_ok(db))
		{
			print_sql_error(db);
			db.exec(
				"ROLLBACK TO actual_migration_sp;"
				"ROLLBACK;"
			);

			return 1;
		}

		db.exec("RELEASE actual_migration_sp");

		db.exec("SAVEPOINT migration_log_sp");

		std::cout << "logging migration " << entry.path().filename().string() << "\n";
		log_migration(db, entry);

		if (!sql::status::is_ok(db))
		{
			print_sql_error(db);
			db.exec(
				"ROLLBACK TO migration_log_sp;"
				"ROLLBACK;"
			);

			return 1;
		}

		db.exec(
			"RELEASE migration_log_sp;"
			"COMMIT;"
		);
	}

	return 0;
}
