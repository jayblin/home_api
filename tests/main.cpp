#include "db/db.hpp"
#include "local/cmake_vars.h"
#include "server/server.hpp"
#include "sock/buffer.hpp"
#include "sock/socket_factory.hpp"
#include "sock/utils.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/json_string_result.hpp"
#include "sqlw/statement.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string_view>
#include <thread>
#include <unordered_map>

static void
    sqlite_error_log_callback(void* pArg, int iErrCode, const char* zMsg)
{
	std::cout << "SQLITE ERROR [" << iErrCode << "] " << zMsg << '\n';
}

/**
 * @see
 * https://github.com/google/googletest/blob/main/docs/advanced.md#sharing-resources-between-tests-in-the-same-test-suite
 */
class AppTest : public ::testing::Test
{
public:
	static auto get_client()
	{
		using namespace std::chrono_literals;

		auto m_client = sock::SocketFactory::instance().create({
		    .domain = sock::Domain::UNSPEC,
		    .type = sock::Type::STREAM,
		    .protocol = sock::Protocol::TCP,
		});

		return m_client;
	}

	static void SetUpTestSuite()
	{
		sqlite3_config(SQLITE_CONFIG_LOG, sqlite_error_log_callback, nullptr);
		// initialize database
		db::migration::migrate(
		    std::filesystem::path {PROJECT_DIR}
		        / "tests/resources/db/app_test.db",
		    MIGRATIONS_DIR
		);

		// init_server
		static std::thread m_server_thread {
		    []()
		    {
			    sqlw::Connection con {(std::filesystem::path {PROJECT_DIR}
			                           / "tests/resources/db/app_test.db")
			                              .string()};

			    server::start({.host = "localhost", .port = "5541"}, &con);
		    }};
		m_server_thread.detach();
	}
};

TEST_F(AppTest, get_foods)
{
	std::vector<std::string> responses;

	auto client = AppTest::get_client();

	std::thread client_thread {
	    [&client, &responses]
	    {
		    using namespace std::chrono_literals;

		    auto& srvr = server::Server::instance();
		    sqlw::Statement stmt {srvr.db_connection};

		    stmt("BEGIN EXCLUSIVE TRANSACTION");

		    stmt(
		        "INSERT INTO food (id,title) VALUES (1,'Vex milk');"
		        "INSERT INTO food (id,title,calories) VALUES (2,'Traveler sparks',400.4);"
		        "INSERT INTO food (id,title,calories,fats) VALUES (3,'Cabal juice',53,103.556);"
		        "INSERT INTO food (id,title,calories,carbohydrates) VALUES (4,'Fallen cereal',340,117.117);"
		        "INSERT INTO food (id,title,proteins) VALUES (5,'Hive strach',554);"
		        "INSERT INTO food (id,title) VALUES (6,'Witness salt');"
		        "INSERT INTO food (id,title) VALUES (7,'Taken bacon');"
		        "INSERT INTO food (id,title) VALUES (8,'Devrim tea');"
		    );

		    sock::Buffer buffer;

		    client.connect({.host = "localhost", .port = "5541"});

		    while (client.status() == sock::Status::CONNECT_ERROR)
		    {
			    std::this_thread::sleep_for(10ms);
			    client.connect({.host = "localhost", .port = "5541"});
		    }

		    client.send("GET /api/foods?limit=4 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/foods?limit=4&page=2 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/foods/4 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/foods?ids=1,3,8,2 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/foods/filter?title=ex milk HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send(
		        "GET /api/foods/filter?calories=<=200&calories=>400 HTTP/1.1\r\n"
		        "User-Agent: gtest\r\n"
		        "Connection: keep-alive\r\n"
		        "Host: localhost\r\n"
		        "\r\n"
		    );

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    stmt("ROLLBACK");
	    }};

	client_thread.join();

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":1,"title":"Vex milk","calories":null,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":2,"title":"Traveler sparks","calories":400.4,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":3,"title":"Cabal juice","calories":53,"proteins":null,"carbohydrates":null,"fats":103.556},)"
	    R"({"id":4,"title":"Fallen cereal","calories":340,"proteins":null,"carbohydrates":117.117,"fats":null})"
	    "]}",
	    responses[1]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":5,"title":"Hive strach","calories":null,"proteins":554,"carbohydrates":null,"fats":null},)"
	    R"({"id":6,"title":"Witness salt","calories":null,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":7,"title":"Taken bacon","calories":null,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":8,"title":"Devrim tea","calories":null,"proteins":null,"carbohydrates":null,"fats":null})"
	    "]}",
	    responses[3]
	);

	ASSERT_EQ(
	    R"({"data":)"
	    R"({"id":4,"title":"Fallen cereal","calories":340,"proteins":null,"carbohydrates":117.117,"fats":null})"
	    "}",
	    responses[5]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":1,"title":"Vex milk","calories":null,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":2,"title":"Traveler sparks","calories":400.4,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":3,"title":"Cabal juice","calories":53,"proteins":null,"carbohydrates":null,"fats":103.556},)"
	    R"({"id":8,"title":"Devrim tea","calories":null,"proteins":null,"carbohydrates":null,"fats":null})"
	    "]}",
	    responses[7]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":1,"title":"Vex milk","calories":null,"proteins":null,"carbohydrates":null,"fats":null})"
	    "]}",
	    responses[9]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":2,"title":"Traveler sparks","calories":400.4,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":3,"title":"Cabal juice","calories":53,"proteins":null,"carbohydrates":null,"fats":103.556})"
	    "]}",
	    responses[11]
	);
}

TEST_F(AppTest, post_foods)
{
	auto client = AppTest::get_client();

	std::vector<std::string> responses;

	std::unordered_map<std::string, std::string> food_values {};
	std::unordered_map<std::string, std::string> user_food_log_values {};

	std::thread client_thread {
	    [&client, &food_values, &responses, &user_food_log_values]
	    {
		    using namespace std::chrono_literals;

		    auto& srvr = server::Server::instance();
		    sqlw::Statement stmt {srvr.db_connection};

		    stmt("BEGIN EXCLUSIVE TRANSACTION");

		    stmt("INSERT INTO user (id,name,created_at)"
		         " VALUES (1,'Clovis','2023-04-25 09:50:31.333')");

		    client.connect({.host = "localhost", .port = "5541"});

		    while (client.status() == sock::Status::CONNECT_ERROR)
		    {
			    std::this_thread::sleep_for(10ms);
			    client.connect({.host = "localhost", .port = "5541"});
		    }

		    sock::Buffer buffer;

		    // send unauthorized request.
		    client.send("POST /api/foods HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Content-Length: 93\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.send(
		        R"({"title":"Darkness mushroom","calories":133.4,"proteins":30,"carbohydrates":45,"fats":11.982})"
		    );

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    buffer.reset();

		    // send autohorized request

		    client.send("POST /api/foods HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "Content-Length: 93\r\n"
		                "Authorization: Basic Q2xvdmlzOg==\r\n"
		                "\r\n"
		                R"({"titl)");

		    client.send(
		        R"(e":"Darkness mushroom","calories":133.4,"proteins":30,"carbohydrates":45,"fats":11.982})"
		    );

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    buffer.reset();

		    client.send("POST /api/foods HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Content-Length: 93\r\n"
		                "Host: localhost\r\n"
		                "Authorization: Basic Q2xvdmlzOg==\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    buffer.reset();

		    client.send(
		        "POST /api/foods HTTP/1.1\r\n"
		        "User-Agent: gtest\r\n"
		        "Connection: keep-alive\r\n"
		        "Content-Length: 93\r\n"
		        "Host: localhost\r\n"
		        "Authorization: Basic Q2xvdmlzOg==\r\n"
		        "\r\n"
		        R"({"title":"Darkness mushroom","calories":11,"proteins":30,"carbohydrates":45,"fats":11.982})"
		    );

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    buffer.reset();

		    client.send(
		        "POST /api/foods HTTP/1.1\r\n"
		        "User-Agent: gtest\r\n"
		        "Connection: keep-alive\r\n"
		        "Content-Length: 93\r\n"
		        "Host: localhost\r\n"
		        "Authorization: Basic Q2xvdmlzOg==\r\n"
		        "\r\n"
		        R"({"title":"Deep pickle","calories":"not a number","proteins":30,"carbohydrates":45,"fats":11.982})"
		    );

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    stmt(
		        "SELECT * FROM food",
		        [&food_values](auto a)
		        {
			        food_values[std::string {a.column_name}] =
			            std::string {a.column_value};
		        }
		    );

		    stmt(
		        "SELECT * FROM user_food_log",
		        [&user_food_log_values](auto a)
		        {
			        user_food_log_values[std::string {a.column_name}] =
			            std::string {a.column_value};
		        }
		    );

		    stmt("ROLLBACK");
	    }};

	client_thread.join();

	/* std::cout << "Printing responses: \n"; */
	/* size_t i = 0; */
	/* for (const auto& r : responses) */
	/* { */
	/* 	std::cout << '[' << i++ << "] " << r << '\n'; */
	/* } */

	ASSERT_STREQ("1", food_values["id"].c_str());
	ASSERT_STREQ("Darkness mushroom", food_values["title"].c_str());
	ASSERT_STREQ("133.4", food_values["calories"].c_str());
	ASSERT_STREQ("30", food_values["proteins"].c_str());
	ASSERT_STREQ("45", food_values["carbohydrates"].c_str());
	ASSERT_STREQ("11.982", food_values["fats"].c_str());

	ASSERT_STREQ("1", user_food_log_values["food_id"].c_str());
	ASSERT_STREQ("1", user_food_log_values["user_id"].c_str());

	ASSERT_EQ(9, responses.size());

	ASSERT_TRUE(
	    responses[0].substr(0, 25).compare("HTTP/1.1 401 Unauthorized") == 0
	) << "First request should not succeed because client is not authorized.\n"
	  << responses[0];

	ASSERT_TRUE(responses[1].substr(0, 15).compare("HTTP/1.1 200 OK") == 0)
	    << "Second request should succeed" << responses[1];

	ASSERT_EQ(
	    R"({"data":{"id":1,"title":"Darkness mushroom","calories":133.4,"proteins":30,"carbohydrates":45,"fats":11.982}})",
	    responses[2]
	) << "In the second part of the response to the second request there should be info about newly created food."
	  << responses[2];

	ASSERT_TRUE(
	    responses[3].substr(0, 24).compare("HTTP/1.1 400 Bad Request") == 0
	)
	    << "Third request should not succeed" << responses[3];

	ASSERT_EQ(
	    R"({"errors":[{"detail":"Erroneous reqeuest body"}]})",
	    responses[4]
	) << "Second part of the third request should not return any data."
	  << responses[4];

	ASSERT_TRUE(
	    responses[5].substr(0, 24).compare("HTTP/1.1 400 Bad Request") == 0
	) << "Fourth request should not succeed because that food already exists."
	  << responses[5];

	ASSERT_TRUE(
	    responses[7].substr(0, 24).compare("HTTP/1.1 400 Bad Request") == 0
	) << "Fourth request should not succeed because calories is not a number."
	  << responses[7];
}

TEST_F(AppTest, get_recipes)
{
	std::vector<std::string> responses;

	auto client = AppTest::get_client();

	std::thread client_thread {
	    [&client, &responses]
	    {
		    using namespace std::chrono_literals;

		    auto& srvr = server::Server::instance();
		    sqlw::Statement stmt {srvr.db_connection};

		    stmt("BEGIN EXCLUSIVE TRANSACTION");

		    stmt(
		        "INSERT INTO cooking_action (id,title)"
		        " VALUES (1,'Mix'),(2,'Boil'),(3,'Fry'),(4,'Season');"

		        "INSERT INTO food (id,title,calories,proteins,carbohydrates,fats)"
		        " VALUES"
		        " (1,'Cereal',340,0.5,60,4),"
		        " (2,'Milk',60,1,50,10),"
		        " (3,'Cereal + milk',NULL,NULL,NULL,NULL),"

		        " (4,'Eggs',300,50,0,30),"
		        " (5,'Butter',600,1,0,99),"
		        " (6,'Salt',NULL,NULL,NULL,NULL),"
		        " (7,'Scrambo',NULL,NULL,NULL,NULL),"

		        " (8,'Water',0,0,0,0),"
		        " (9,'Giga-leaf',0.4,NULL,NULL,NULL),"
		        " (10,'Chai',NULL,NULL,NULL,NULL),"

		        " (11,'Chicken',120,25,3,12),"
		        " (12,'Chicken broth',NULL,NULL,NULL,NULL),"

		        " (13,'Noodles',340,10,2,55),"
		        " (14,'Chicken noodle soup',NULL,NULL,NULL,NULL);"

		        "INSERT INTO recipe_step (id,recipe_id,cooking_action_id,priority)"
		        " VALUES"
		        " (1,3,1,0)," // cereal + milk

		        " (2,7,3,0)," // scrambo
		        " (3,7,4,1),"

		        " (4,10,2,0)," // chai
		        " (5,10,1,1),"

		        " (6,12,2,0)," // chicken broth

		        " (7,14,1,0);" // chicken noodle soup

		        "INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass)"
		        " VALUES"
		        " (1,1,40)," // ceral + milk
		        " (1,2,100),"

		        " (2,4,100)," // scrambo
		        " (2,5,20),"
		        " (3,6,2),"

		        " (4,8,1000)," // chai
		        " (5,9,5),"

		        " (6,8,1000)," // chicken broth
		        " (6,11,500),"

		        " (7,13,200)," // chicken noodle soup
		        " (7,12,1000);"
		    );

		    client.connect({.host = "localhost", .port = "5541"});

		    while (client.status() == sock::Status::CONNECT_ERROR)
		    {
			    std::this_thread::sleep_for(10ms);
			    client.connect({.host = "localhost", .port = "5541"});
		    }

		    sock::Buffer buffer;

		    client.send("GET /api/recipes?limit=3 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/recipes?limit=2&page=3 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/recipes/3 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/recipes/?ids=10,12,3 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    stmt("ROLLBACK");
	    }};

	client_thread.join();

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"food":3,"steps":[0,0],"actions":[1,1],"foods":[1,2]},)"
	    R"({"food":7,"steps":[0,0,1],"actions":[3,3,4],"foods":[4,5,6]},)"
	    R"({"food":10,"steps":[0,1],"actions":[2,1],"foods":[8,9]})"
	    "]}",
	    responses[1]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"food":14,"steps":[0,0],"actions":[1,1],"foods":[13,12]})"
	    "]}",
	    responses[3]
	);

	ASSERT_EQ(
	    R"({"data":)"
	    R"({"food":3,"steps":[0,0],"actions":[1,1],"foods":[1,2]})"
	    "}",
	    responses[5]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"food":3,"steps":[0,0],"actions":[1,1],"foods":[1,2]},)"
	    R"({"food":10,"steps":[0,1],"actions":[2,1],"foods":[8,9]},)"
	    R"({"food":12,"steps":[0,0],"actions":[2,2],"foods":[8,11]})"
	    "]}",
	    responses[7]
	);
}

TEST_F(AppTest, get_cooking_actions)
{
	std::vector<std::string> responses;

	auto client = AppTest::get_client();

	std::thread client_thread {
	    [&client, &responses]
	    {
		    using namespace std::chrono_literals;

		    auto& srvr = server::Server::instance();
		    sqlw::Statement stmt {srvr.db_connection};

		    stmt("BEGIN EXCLUSIVE TRANSACTION");

		    stmt(
		        "INSERT INTO cooking_action (id,title,description)"
		        " VALUES (1,'Mix','Do mixin'),(2,'Boil',NULL),(3,'Fry',NULL),(4,'Season',NULL);"
		    );

		    client.connect({.host = "localhost", .port = "5541"});

		    while (client.status() == sock::Status::CONNECT_ERROR)
		    {
			    std::this_thread::sleep_for(10ms);
			    client.connect({.host = "localhost", .port = "5541"});
		    }

		    sock::Buffer buffer;

		    client.send("GET /api/cooking_actions?limit=3 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/cooking_actions?limit=3&page=2 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/cooking_actions/1 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send("GET /api/cooking_actions/?ids=4,1,3 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.send(
		        "GET /api/cooking_actions/filter?title=boil HTTP/1.1\r\n"
		        "User-Agent: gtest\r\n"
		        "Connection: keep-alive\r\n"
		        "Host: localhost\r\n"
		        "\r\n"
		    );

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    stmt("ROLLBACK");
	    }};

	client_thread.join();

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":1,"title":"Mix","description":"Do mixin"},)"
	    R"({"id":2,"title":"Boil","description":null},)"
	    R"({"id":3,"title":"Fry","description":null})"
	    "]}",
	    responses[1]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":4,"title":"Season","description":null})"
	    "]}",
	    responses[3]
	);

	ASSERT_EQ(
	    R"({"data":)"
	    R"({"id":1,"title":"Mix","description":"Do mixin"})"
	    "}",
	    responses[5]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":1,"title":"Mix","description":"Do mixin"},)"
	    R"({"id":3,"title":"Fry","description":null},)"
	    R"({"id":4,"title":"Season","description":null})"
	    "]}",
	    responses[7]
	);

	ASSERT_EQ(
	    R"({"data":[)"
	    R"({"id":2,"title":"Boil","description":null})"
	    "]}",
	    responses[9]
	);
}

TEST_F(AppTest, post_cooking_action)
{
	auto client = AppTest::get_client();

	std::vector<std::string> responses;

	std::unordered_map<std::string, std::string> action_values {};

	std::thread client_thread {
	    [&client, &action_values, &responses]
	    {
		    using namespace std::chrono_literals;

		    auto& srvr = server::Server::instance();
		    sqlw::Statement stmt {srvr.db_connection};

		    stmt("BEGIN EXCLUSIVE TRANSACTION");

		    stmt("INSERT INTO user (id,name,created_at)"
		         " VALUES (1,'Clovis','2023-04-25 09:50:31.333')");

		    client.connect({.host = "localhost", .port = "5541"});

		    while (client.status() == sock::Status::CONNECT_ERROR)
		    {
			    std::this_thread::sleep_for(10ms);
			    client.connect({.host = "localhost", .port = "5541"});
		    }

		    sock::Buffer buffer;

		    // send unauthorized request.
		    client.send("POST /api/cooking_actions HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Content-Length: 93\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.send(R"({"title":"Fry","description":"Do a frying"})");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    buffer.reset();

		    // send autohorized request

		    client.send("POST /api/cooking_actions HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "Content-Length: 93\r\n"
		                "Authorization: Basic Q2xvdmlzOg==\r\n"
		                "\r\n"
		                R"({"titl)");

		    client.send(R"(e":"Fry","description":"Do a fryin'"})");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    buffer.reset();

		    client.send("POST /api/cooking_actions HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Content-Length: 93\r\n"
		                "Host: localhost\r\n"
		                "Authorization: Basic Q2xvdmlzOg==\r\n"
		                "\r\n");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    buffer.reset();

		    client.send("POST /api/cooking_actions HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Content-Length: 93\r\n"
		                "Host: localhost\r\n"
		                "Authorization: Basic Q2xvdmlzOg==\r\n"
		                "\r\n"
		                R"({"title":"Fry","description":"Do a boiling"})");

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    client.receive(buffer);
		    responses.push_back(std::string {buffer.view()});

		    stmt(
		        "SELECT * FROM cooking_action",
		        [&action_values](auto a)
		        {
			        action_values[std::string {a.column_name}] =
			            std::string {a.column_value};
		        }
		    );

		    stmt("ROLLBACK");
	    }};

	client_thread.join();

	ASSERT_STREQ("1", action_values["id"].c_str());
	ASSERT_STREQ("Fry", action_values["title"].c_str());
	ASSERT_STREQ("Do a fryin'", action_values["description"].c_str());

	ASSERT_EQ(7, responses.size());

	ASSERT_TRUE(
	    responses[0].substr(0, 25).compare("HTTP/1.1 401 Unauthorized") == 0
	) << "First request should not succeed because client is not authorized.\n"
	  << responses[0];

	ASSERT_TRUE(responses[1].substr(0, 15).compare("HTTP/1.1 200 OK") == 0)
	    << "Second request should succeed" << responses[1];

	ASSERT_EQ(
	    R"({"data":{"id":1,"title":"Fry","description":"Do a fryin'"}})",
	    responses[2]
	) << "In the second part of the response to the second request there should be info about newly created action."
	  << responses[2];

	ASSERT_TRUE(
	    responses[3].substr(0, 24).compare("HTTP/1.1 400 Bad Request") == 0
	)
	    << "Third request should not succeed" << responses[3];

	ASSERT_EQ(
	    R"({"errors":[{"detail":"Erroneous reqeuest body"}]})",
	    responses[4]
	) << "Second part of the third request should not return any data."
	  << responses[4];

	ASSERT_TRUE(
	    responses[5].substr(0, 24).compare("HTTP/1.1 400 Bad Request") == 0
	) << "Fourth request should not succeed because that action already exists."
	  << responses[5];
}
