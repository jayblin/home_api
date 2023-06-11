#include "db/db.hpp"
#include "local/cmake_vars.h"
#include "server/server.hpp"
#include "sock/buffer.hpp"
#include "sock/socket_factory.hpp"
#include "sock/utils.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
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
	sock::Buffer buffer_1;
	sock::Buffer buffer_2;
	sock::Buffer buffer_3;

	auto client = AppTest::get_client();

	std::thread client_thread {
	    [&client, &buffer_1, &buffer_2, &buffer_3]
	    {
		    using namespace std::chrono_literals;

		    auto& srvr = server::Server::instance();
		    sqlw::Statement stmt {srvr.db_connection};

		    stmt("BEGIN EXCLUSIVE TRANSACTION");

		    stmt(
		        "INSERT INTO food (id,title) VALUES (1,'Vex milk');"
		        "INSERT INTO food (id,title,calories) VALUES (2,'Traveler sparks',400.4);"
		        "INSERT INTO food (id,title,fats) VALUES (3,'Cabal juice',103.556);"
		        "INSERT INTO food (id,title,carbohydrates) VALUES (4,'Fallen cereal',117.117);"
		        "INSERT INTO food (id,title,proteins) VALUES (5,'Hive strach',554);"
		        "INSERT INTO food (id,title) VALUES (6,'Witness salt');"
		        "INSERT INTO food (id,title) VALUES (7,'Taken bacon');"
		        "INSERT INTO food (id,title) VALUES (8,'Devrim tea');"
		    );

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

		    client.receive(buffer_1);
		    client.receive(buffer_1);

		    client.send("GET /api/foods?limit=4&page=2 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer_2);
		    client.receive(buffer_2);

		    client.send("GET /api/foods/4 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer_3);
		    client.receive(buffer_3);

		    stmt("ROLLBACK");
	    }};

	client_thread.join();

	ASSERT_STREQ(
	    R"({"data":[)"
	    R"({"id":1,"title":"Vex milk","calories":null,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":2,"title":"Traveler sparks","calories":400.4,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":3,"title":"Cabal juice","calories":null,"proteins":null,"carbohydrates":null,"fats":103.556},)"
	    R"({"id":4,"title":"Fallen cereal","calories":null,"proteins":null,"carbohydrates":117.117,"fats":null})"
	    "]}",
	    buffer_1.view().data()
	);

	ASSERT_STREQ(
	    R"({"data":[)"
	    R"({"id":5,"title":"Hive strach","calories":null,"proteins":554,"carbohydrates":null,"fats":null},)"
	    R"({"id":6,"title":"Witness salt","calories":null,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":7,"title":"Taken bacon","calories":null,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":8,"title":"Devrim tea","calories":null,"proteins":null,"carbohydrates":null,"fats":null})"
	    "]}",
	    buffer_2.view().data()
	);

	ASSERT_STREQ(
	    R"({"data":)"
	    R"({"id":4,"title":"Fallen cereal","calories":null,"proteins":null,"carbohydrates":117.117,"fats":null})"
	    "}",
	    buffer_3.view().data()
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

		    stmt(
		        "INSERT INTO user (id,name,created_at)"
				" VALUES (1,'Clovis','2023-04-25 09:50:31.333')"
		    );

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
		                "\r\n"
					);

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

		    client.send("POST /api/foods HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
						"Content-Length: 93\r\n"
		                "Host: localhost\r\n"
		                "Authorization: Basic Q2xvdmlzOg==\r\n"
		                "\r\n"
						R"({"title":"Darkness mushroom","calories":11,"proteins":30,"carbohydrates":45,"fats":11.982})");

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
		                "\r\n"
						R"({"title":"Deep pickle","calories":"not a number","proteins":30,"carbohydrates":45,"fats":11.982})");

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

	ASSERT_TRUE(responses[0].substr(0, 25).compare("HTTP/1.1 401 Unauthorized") == 0)
	    << "First request should not succeed because client is not authorized.\n" << responses[0];

	ASSERT_TRUE(responses[1].substr(0, 15).compare("HTTP/1.1 200 OK") == 0)
	    << "Second request should succeed" << responses[1];

	ASSERT_EQ(
		R"({"data":{"id":1,"title":"Darkness mushroom","calories":133.4,"proteins":30,"carbohydrates":45,"fats":11.982}})",
	    responses[2]
	) << "In the second part of the response to the second request there should be info about newly created food." << responses[2];

	ASSERT_TRUE(
	    responses[3].substr(0, 24).compare("HTTP/1.1 400 Bad Request") == 0
	) << "Third request should not succeed" << responses[3];

	ASSERT_EQ(R"({"errors":[{"detail":"Erroneous reqeuest body"}]})", responses[4])
	    << "Second part of the third request should not return any data." << responses[4];

	ASSERT_TRUE(
	    responses[5].substr(0, 24).compare("HTTP/1.1 400 Bad Request") == 0
	) << "Fourth request should not succeed because that food already exists." << responses[5];

	ASSERT_TRUE(
	    responses[7].substr(0, 24).compare("HTTP/1.1 400 Bad Request") == 0
	) << "Fourth request should not succeed because calories is not a number." << responses[7];
}
