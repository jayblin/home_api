#include "local/cmake_vars.h"
#include "server/server.hpp"
#include "sqlw/connection.hpp"
#include <iostream>

static void
    sqlite_error_log_callback(void* pArg, int iErrCode, const char* zMsg)
{
	std::cout << "SQLITE ERROR [" << iErrCode << ']' << zMsg << '\n';
}

int main(int argc, char const* argv[])
{
	sqlite3_config(SQLITE_CONFIG_LOG, sqlite_error_log_callback, nullptr);

	sqlw::Connection con {(std::filesystem::path {DATABASE_NAME}
	                       / "tests/resources/db/app_test.db")
	                          .string()};

	server::start({.host = "localhost", .port = "5541"}, &con);

	return 0;
}
