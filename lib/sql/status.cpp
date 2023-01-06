#include "sql/status.hpp"
#include "sql/sqlite3.hpp"
#include "sql/statement.hpp"
#include <sstream>
#include <string_view>

std::string sql::status::verbose(const sql::Sqlite3& sqlite)
{
	return verbose(sqlite.status());
}

std::string sql::status::verbose(const sql::Statement& stmt)
{
	return verbose(stmt.status());
}

static std::string _verbose(const sql::status::Code code)
{
	switch (code)
	{
		case sql::status::Code::OK:
			return "Successful result";
		case sql::status::Code::ERROR:
			return "Generic error";
		case sql::status::Code::INTERNAL:
			return "Internal logic error in SQLite";
		case sql::status::Code::PERM:
			return "Access permission denied";
		case sql::status::Code::ABORT:
			return "Callback routine requested an abort";
		case sql::status::Code::BUSY:
			return "The database file is locked";
		case sql::status::Code::LOCKED:
			return "A table in the database is locked";
		case sql::status::Code::NOMEM:
			return "A malloc() failed";
		case sql::status::Code::READONLY:
			return "Attempt to write a readonly database";
		case sql::status::Code::INTERRUPT:
			return "Operation terminated by sqlite3_interrupt(";
		case sql::status::Code::IOERR:
			return "Some kind of disk I/O error occurred";
		case sql::status::Code::CORRUPT:
			return "The database disk image is malformed";
		case sql::status::Code::NOTFOUND:
			return "Unknown opcode in sqlite3_file_control()";
		case sql::status::Code::FULL:
			return "Insertion failed because database is full";
		case sql::status::Code::CANTOPEN:
			return "Unable to open the database file";
		case sql::status::Code::PROTOCOL:
			return "Database lock protocol error";
		case sql::status::Code::EMPTY:
			return "Internal use only";
		case sql::status::Code::SCHEMA:
			return "The database schema changed";
		case sql::status::Code::TOOBIG:
			return "String or BLOB exceeds size limit";
		case sql::status::Code::CONSTRAINT:
			return "Abort due to constraint violation";
		case sql::status::Code::MISMATCH:
			return "Data type mismatch";
		case sql::status::Code::MISUSE:
			return "Library used incorrectly";
		case sql::status::Code::NOLFS:
			return "Uses OS features not supported on host";
		case sql::status::Code::AUTH:
			return "Authorization denied";
		case sql::status::Code::FORMAT:
			return "Not used";
		case sql::status::Code::RANGE:
			return "2nd parameter to sqlite3_bind out of range";
		case sql::status::Code::NOTADB:
			return "File opened that is not a database file";
		case sql::status::Code::NOTICE:
			return "Notifications from sqlite3_log()";
		case sql::status::Code::WARNING:
			return "Warnings from sqlite3_log()";
		default:
			return "Unknown sqlite error code";
	}
}

std::string sql::status::verbose(const sql::status::Code code)
{
	std::stringstream ss;

	ss << '(' << static_cast<int>(code) << ") "
		<< sql::status::view(code) << ": "
		<< _verbose(code)
	;

	return ss.str();
}


bool sql::status::is_ok(const Sqlite3& sqlite)
{
	return is_ok(sqlite.status());
}

bool sql::status::is_ok(const Statement& stmt)
{
	return is_ok(stmt.status());
}

bool sql::status::is_ok(const status::Code code)
{
	using C = sql::status::Code;

	return code == C::OK || code == C::DONE || code == C::ROW;
}

