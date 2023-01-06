#ifndef LIB_SQL_STATUS_H_
#define LIB_SQL_STATUS_H_

#include "sql/forward.hpp"
#include "sql/sqlite3.hpp"
#include "sql/statement.hpp"
#include <string_view>

namespace sql::status
{
	auto code(const Sqlite3&) -> Code;
	auto code(const Statement&) -> Code;

	constexpr auto view(const Code code) -> std::string_view
	{
		switch (code)
		{
			case status::Code::OK:
				return "OK";
			case status::Code::ERROR:
				return "ERROR";
			case status::Code::INTERNAL:
				return "INTERNAL";
			case status::Code::PERM:
				return "PERM";
			case status::Code::ABORT:
				return "ABORT";
			case status::Code::BUSY:
				return "BUSY";
			case status::Code::LOCKED:
				return "LOCKED";
			case status::Code::NOMEM:
				return "NOMEM";
			case status::Code::READONLY:
				return "READONLY";
			case status::Code::INTERRUPT:
				return "INTERRUPT";
			case status::Code::IOERR:
				return "IOERR|ROW|DONE";
			case status::Code::CORRUPT:
				return "CORRUPT";
			case status::Code::NOTFOUND:
				return "NOTFOUND";
			case status::Code::FULL:
				return "FULL";
			case status::Code::CANTOPEN:
				return "CANTOPEN";
			case status::Code::PROTOCOL:
				return "PROTOCOL";
			case status::Code::EMPTY:
				return "EMPTY";
			case status::Code::SCHEMA:
				return "SCHEMA";
			case status::Code::TOOBIG:
				return "TOOBIG";
			case status::Code::CONSTRAINT:
				return "CONSTRAINT";
			case status::Code::MISMATCH:
				return "MISMATCH";
			case status::Code::MISUSE:
				return "MISUSE";
			case status::Code::NOLFS:
				return "NOLFS";
			case status::Code::AUTH:
				return "AUTH";
			case status::Code::FORMAT:
				return "FORMAT";
			case status::Code::RANGE:
				return "RANGE";
			case status::Code::NOTADB:
				return "NOTADB";
			case status::Code::NOTICE:
				return "NOTICE";
			case status::Code::WARNING:
				return "WARNING";
			default:
				return "UNKNOWN";
		}
	}
	constexpr auto view(const Sqlite3& db) -> std::string_view
	{
		return view(db.status());
	}
	constexpr auto view(const Statement& stmt) -> std::string_view
	{
		return view(stmt.status());
	}

	auto verbose(const Code) -> std::string;
	auto verbose(const Sqlite3&) -> std::string;
	auto verbose(const Statement&) -> std::string;

	auto is_ok(const Code) -> bool;
	auto is_ok(const Sqlite3&) -> bool;
	auto is_ok(const Statement&) -> bool;
}

#endif // LIB_SQL_STATUS_H_
