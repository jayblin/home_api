#include "db/db.hpp"
#include "local/cmake_vars.h"
#include "server/server.hpp"
/* #include "http/parsor.hpp" */
/* #include "local/cmake_vars.h" */
#include <iostream>
#include <string_view>
#include <thread>
/* #include "local/route_map.hpp" */
#include "sock/buffer.hpp"
#include "sock/socket_factory.hpp"
#include "sock/utils.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
/* #include "sqlw/connection.hpp" */
/* #include <filesystem> */
#include <gtest/gtest.h>

static void
    sqlite_error_log_callback(void* pArg, int iErrCode, const char* zMsg)
{
	std::cout << '[' << iErrCode << ']' << zMsg << '\n';
}

/**
 * @see
 * https://github.com/google/googletest/blob/main/docs/advanced.md#sharing-resources-between-tests-in-the-same-test-suite
 */
class AppTest : public ::testing::Test
{
public:
	static auto& get_client()
	{
		static auto m_client = sock::SocketFactory::instance().create({
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

	auto& client = AppTest::get_client();

	std::thread client_thread {
	    [&client, &buffer_1, &buffer_2]
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
		    }

		    client.send("GET /api/foods?limit=4 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer_1);
			/* std::cout << "{{" << buffer_1.view() << "}}" << '\n'; */
		    client.receive(buffer_1);
			/* std::cout << "{{" << buffer_1.view() << "}}" << '\n'; */

		    client.send("GET /api/foods?limit=4&page=2 HTTP/1.1\r\n"
		                "User-Agent: gtest\r\n"
		                "Connection: keep-alive\r\n"
		                "Host: localhost\r\n"
		                "\r\n");

		    client.receive(buffer_2);
			/* std::cout << "{{" << buffer_2.view() << "}}" << '\n'; */
		    client.receive(buffer_2);
			/* std::cout << "{{" << buffer_2.view() << "}}" << '\n'; */

		    stmt("ROLLBACK");
	    }};

	client_thread.join();

	ASSERT_STREQ(
	    "["
	    R"({"id":1,"title":"Vex milk","calories":null,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":2,"title":"Traveler sparks","calories":400.4,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":3,"title":"Cabal juice","calories":null,"proteins":null,"carbohydrates":null,"fats":103.556},)"
	    R"({"id":4,"title":"Fallen cereal","calories":null,"proteins":null,"carbohydrates":117.117,"fats":null})"
	    "]",
	    buffer_1.view().data()
	);

	ASSERT_STREQ(
	    "["
	    R"({"id":5,"title":"Hive strach","calories":null,"proteins":554,"carbohydrates":null,"fats":null},)"
	    R"({"id":6,"title":"Witness salt","calories":400.4,"proteins":null,"carbohydrates":null,"fats":null},)"
	    R"({"id":7,"title":"Taken bacon","calories":null,"proteins":null,"carbohydrates":null,"fats":103.556},)"
	    R"({"id":8,"title":"Devrim tea","calories":null,"proteins":null,"carbohydrates":117.117,"fats":null})"
	    "]",
	    buffer_2.view().data()
	);
}
