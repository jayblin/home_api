#ifndef SERVER_H_
#define SERVER_H_

#include "local/cmake_vars.h"
#include "sock/utils.hpp"
#include "sqlw/connection.hpp"
#include <filesystem>

namespace server
{
	class Server
	{
	public:
		static Server& instance()
		{
			static Server _instance;

			return _instance;
		}

		sqlw::Connection* db_connection = nullptr;

		Server(const Server&) = delete;
		Server& operator=(const Server&) = delete;

	private:
		Server() {};
		/* { */
		/* 	db_connection.connect( */
		/* 	    (std::filesystem::path {DATABASE_DIR} / DATABASE_NAME).string() */
		/* 	); */
		/* } */
	};

	/* struct Server */
	/* { */
	/* 	sqlw::Connection& db_connection; */
	/* }; */

	void start(sock::Address, sqlw::Connection*);
} // namespace server

#endif // SERVER_H_
