#include "sql/sqlite3.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

sql::Sqlite3::Sqlite3(std::string_view&& file_name)
{
	auto rc = sqlite3_open(file_name.data(), &m_db);

	if (rc)
	{
		sqlite3_close(m_db);
	}
}

sql::Sqlite3::~Sqlite3()
{
	sqlite3_close(m_db);
}

void sql::Sqlite3::exec(
    std::string_view&& sql,
    sqlite3_callback callback,
    void* obj
)
{
	auto rc = sqlite3_exec(m_db, sql.data(), callback, obj, &m_err_msg);

	m_status = static_cast<Sqlite3::Status>(rc);
}

std::string_view sql::status_str(sql::Sqlite3::Status status)
{
	switch (status)
	{
			/* Successful result */
		case sql::Sqlite3::Status::OK:
			return "OK";
		/* Generic error */
		case sql::Sqlite3::Status::ERROR:
			return "ERROR";
		/* Internal logic error in SQLite */
		case sql::Sqlite3::Status::INTERNAL:
			return "INTERNAL";
		/* Access permission denied */
		case sql::Sqlite3::Status::PERM:
			return "PERM";
		/* Callback routine requested an abort */
		case sql::Sqlite3::Status::ABORT:
			return "ABORT";
		/* The database file is locked */
		case sql::Sqlite3::Status::BUSY:
			return "BUSY";
		/* A table in the database is locked */
		case sql::Sqlite3::Status::LOCKED:
			return "LOCKED";
		/* A malloc() failed */
		case sql::Sqlite3::Status::NOMEM:
			return "NOMEM";
		/* Attempt to write a readonly database */
		case sql::Sqlite3::Status::READONLY:
			return "READONLY";
		/* Operation terminated by sqlite3_interrupt()*/
		case sql::Sqlite3::Status::INTERRUPT:
			return "INTERRUPT";
		/* Some kind of disk I/O error occurred */
		case sql::Sqlite3::Status::IOERR:
			return "IOERR|ROW|DONE";
		/* The database disk image is malformed */
		case sql::Sqlite3::Status::CORRUPT:
			return "CORRUPT";
		/* Unknown opcode in sqlite3_file_control() */
		case sql::Sqlite3::Status::NOTFOUND:
			return "NOTFOUND";
		/* Insertion failed because database is full */
		case sql::Sqlite3::Status::FULL:
			return "FULL";
		/* Unable to open the database file */
		case sql::Sqlite3::Status::CANTOPEN:
			return "CANTOPEN";
		/* Database lock protocol error */
		case sql::Sqlite3::Status::PROTOCOL:
			return "PROTOCOL";
		/* Internal use only */
		case sql::Sqlite3::Status::EMPTY:
			return "EMPTY";
		/* The database schema changed */
		case sql::Sqlite3::Status::SCHEMA:
			return "SCHEMA";
		/* String or BLOB exceeds size limit */
		case sql::Sqlite3::Status::TOOBIG:
			return "TOOBIG";
		/* Abort due to constraint violation */
		case sql::Sqlite3::Status::CONSTRAINT:
			return "CONSTRAINT";
		/* Data type mismatch */
		case sql::Sqlite3::Status::MISMATCH:
			return "MISMATCH";
		/* Library used incorrectly */
		case sql::Sqlite3::Status::MISUSE:
			return "MISUSE";
		/* Uses OS features not supported on host */
		case sql::Sqlite3::Status::NOLFS:
			return "NOLFS";
		/* Authorization denied */
		case sql::Sqlite3::Status::AUTH:
			return "AUTH";
		/* Not used */
		case sql::Sqlite3::Status::FORMAT:
			return "FORMAT";
		/* 2nd parameter to sqlite3_bind out of range */
		case sql::Sqlite3::Status::RANGE:
			return "RANGE";
		/* File opened that is not a database file */
		case sql::Sqlite3::Status::NOTADB:
			return "NOTADB";
		/* Notifications from sqlite3_log() */
		case sql::Sqlite3::Status::NOTICE:
			return "NOTICE";
		/* Warnings from sqlite3_log() */
		case sql::Sqlite3::Status::WARNING:
			return "WARNING";
	}
}
