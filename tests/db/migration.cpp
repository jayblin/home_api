#include "db/db.hpp"
#include "local/cmake_vars.h"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <gsl/pointers>
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>

GTEST_TEST(migrate, warns_when_folder_does_not_exist)
{
	const auto db_path = std::filesystem::path {PROJECT_DIR}
	                   / "tests/resources/db_does_not_exist/test.db";

	auto code = db::migration::migrate(
	    db_path.string(),
	    std::filesystem::path {MIGRATIONS_DIR}
	);

	ASSERT_EQ(10, code);
}

GTEST_TEST(migrate, does_execute_all_migrations)
{
	const auto db_path =
	    std::filesystem::path {PROJECT_DIR} / "tests/resources/db/test.db";

	if (std::filesystem::exists(db_path))
	{
		std::filesystem::remove(db_path);
	}

	const auto code = db::migration::migrate(
	    db_path,
	    MIGRATIONS_DIR
	);

	ASSERT_EQ(0, code)
	    << "db::migration::migrate() should return no error codes";

	sqlw::Connection con {db_path.string()};
	sqlw::Statement stmt {&con};

	std::string db_name;
	stmt(
	    "SELECT name from pragma_table_list('migration')",
	    [&db_name](auto args)
	    {
		    if (args.column_name.compare("name") == 0)
		    {
			    db_name = args.column_value;
		    }
	    }
	);

	ASSERT_STREQ("migration", db_name.data())
	    << "There should be a table \"migration\" after executing migrations.";

	const std::array<const std::string, 5> expected_migrations = {
	    "0.sql",
	    "1.sql",
	    "2.sql",
	    "3.sql",
	    "4.sql",
	};
	std::vector<std::string> executed_migrations;
	stmt(
	    "SELECT filename FROM migration",
	    [&executed_migrations](auto args)
	    {
		    executed_migrations.push_back(std::string {args.column_value});
	    }
	);
	ASSERT_EQ(5, executed_migrations.size());

	for (const auto& filename : expected_migrations)
	{
		ASSERT_TRUE(
		    std::find_if(
		        executed_migrations.begin(),
		        executed_migrations.end(),
		        [&filename](auto n)
		        {
			        return n.compare(filename) == 0;
		        }
		    )
		    != executed_migrations.end()
		) << "Migration \"filename\" must be logged in \"migration\" table.";
	}

	stmt("BEGIN EXCLUSIVE TRANSACTION");

	{
		stmt("INSERT INTO food (id,title,calories,proteins,carbohydrates,fats)"
		     " VALUES (1,'salt',40.4,30.1,14.664,5.009)");

		std::tuple<
		    std::string, // id
		    std::string, // title
		    std::string, // calories
		    std::string, // proteins
		    std::string, // carbohydrates
		    std::string> // fats
		    salt;

		stmt(
		    "SELECT * FROM food",
		    [&salt](auto args)
		    {
			    if (args.column_name.compare("id") == 0)
			    {
				    std::get<0>(salt) = args.column_value;
			    }
			    else if (args.column_name.compare("title") == 0)
			    {
				    std::get<1>(salt) = args.column_value;
			    }
			    else if (args.column_name.compare("calories") == 0)
			    {
				    std::get<2>(salt) = args.column_value;
			    }
			    else if (args.column_name.compare("proteins") == 0)
			    {
				    std::get<3>(salt) = args.column_value;
			    }
			    else if (args.column_name.compare("carbohydrates") == 0)
			    {
				    std::get<4>(salt) = args.column_value;
			    }
			    else if (args.column_name.compare("fats") == 0)
			    {
				    std::get<5>(salt) = args.column_value;
			    }
		    }
		);

		ASSERT_STREQ("1", std::get<0>(salt).data());
		ASSERT_STREQ("salt", std::get<1>(salt).data());
		ASSERT_STREQ("40.4", std::get<2>(salt).data());
		ASSERT_STREQ("30.1", std::get<3>(salt).data());
		ASSERT_STREQ("14.664", std::get<4>(salt).data());
		ASSERT_STREQ("5.009", std::get<5>(salt).data());
	}

	{
		stmt("INSERT INTO food (id,title) VALUES (10,'composite');"
		     "INSERT INTO food (id,title) VALUES (20,'composite');"
		     "INSERT INTO food_composition (composite_id, particular_id)"
		     " VALUES (10,20);");

		std::tuple<
		    std::string, // composite_id
		    std::string> // particluar_id
		    composition;

		stmt(
		    "SELECT * FROM food_composition",
		    [&composition](auto args)
		    {
			    if (args.column_name.compare("composite_id") == 0)
			    {
				    std::get<0>(composition) = args.column_value;
			    }
			    else if (args.column_name.compare("particular_id") == 0)
			    {
				    std::get<1>(composition) = args.column_value;
			    }
		    }
		);

		ASSERT_STREQ("10", std::get<0>(composition).data());
		ASSERT_STREQ("20", std::get<1>(composition).data());
	}

	{
		stmt("INSERT INTO user (id,name,created_at)"
		     " VALUES (1,'Commander Zavala','2023-04-25 09:50:31.333');");

		std::tuple<
		    std::string, // id
		    std::string, // name
		    std::string> // created_at
		    user;

		stmt(
		    "SELECT * FROM user",
		    [&user](auto args)
		    {
			    if (args.column_name.compare("id") == 0)
			    {
				    std::get<0>(user) = args.column_value;
			    }
			    else if (args.column_name.compare("name") == 0)
			    {
				    std::get<1>(user) = args.column_value;
			    }
			    else if (args.column_name.compare("created_at") == 0)
			    {
				    std::get<2>(user) = args.column_value;
			    }
		    }
		);

		ASSERT_STREQ("1", std::get<0>(user).data());
		ASSERT_STREQ("Commander Zavala", std::get<1>(user).data());
		ASSERT_STREQ("2023-04-25 09:50:31.333", std::get<2>(user).data());
	}

	{
		stmt("INSERT INTO food (id,title) VALUES (100,'Hive starch');"
		     "INSERT INTO user (id,name,created_at)"
		     " VALUES (10,'Drifter','2023-01-26 19:04:31.333');"
		     "INSERT INTO user_food_log (user_id,food_id,created_at)"
		     " VALUES (10,100,'2023-02-20 05:56:01.333')");

		std::tuple<
		    std::string, // user_id
		    std::string, // food_id
		    std::string> // created_at
		    log;

		stmt(
		    "SELECT * FROM user_food_log",
		    [&log](auto args)
		    {
			    if (args.column_name.compare("user_id") == 0)
			    {
				    std::get<0>(log) = args.column_value;
			    }
			    else if (args.column_name.compare("food_id") == 0)
			    {
				    std::get<1>(log) = args.column_value;
			    }
			    else if (args.column_name.compare("created_at") == 0)
			    {
				    std::get<2>(log) = args.column_value;
			    }
		    }
		);

		ASSERT_STREQ("10", std::get<0>(log).data());
		ASSERT_STREQ("100", std::get<1>(log).data());
		ASSERT_STREQ("2023-02-20 05:56:01.333", std::get<2>(log).data());
	}

	stmt("ROLLBACK");
}
