#include "sqlite3.h"
#include <concepts>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace sql
{
	/**
	 * A wrapper arond sqlite3 functions.
	 */
	class Sqlite3
	{
	   public:
		enum class Status
		{
			/* Successful result */
			OK = 0,
			/* Generic error */
			ERROR = 1,
			/* Internal logic error in SQLite */
			INTERNAL = 2,
			/* Access permission denied */
			PERM = 3,
			/* Callback routine requested an abort */
			ABORT = 4,
			/* The database file is locked */
			BUSY = 5,
			/* A table in the database is locked */
			LOCKED = 6,
			/* A malloc() failed */
			NOMEM = 7,
			/* Attempt to write a readonly database */
			READONLY = 8,
			/* Operation terminated by sqlite3_interrupt()*/
			INTERRUPT = 9,
			/* Some kind of disk I/O error occurred */
			IOERR = 10,
			/* The database disk image is malformed */
			CORRUPT = 11,
			/* Unknown opcode in sqlite3_file_control() */
			NOTFOUND = 12,
			/* Insertion failed because database is full */
			FULL = 13,
			/* Unable to open the database file */
			CANTOPEN = 14,
			/* Database lock protocol error */
			PROTOCOL = 15,
			/* Internal use only */
			EMPTY = 16,
			/* The database schema changed */
			SCHEMA = 17,
			/* String or BLOB exceeds size limit */
			TOOBIG = 18,
			/* Abort due to constraint violation */
			CONSTRAINT = 19,
			/* Data type mismatch */
			MISMATCH = 20,
			/* Library used incorrectly */
			MISUSE = 21,
			/* Uses OS features not supported on host */
			NOLFS = 22,
			/* Authorization denied */
			AUTH = 23,
			/* Not used */
			FORMAT = 24,
			/* 2nd parameter to sqlite3_bind out of range */
			RANGE = 25,
			/* File opened that is not a database file */
			NOTADB = 26,
			/* Notifications from sqlite3_log() */
			NOTICE = 27,
			/* Warnings from sqlite3_log() */
			WARNING = 28,
			/* sqlite3_step() has another row ready */
			ROW = 10,
			/* sqlite3_step() has finished executing */
			DONE = 10,
		};

		Sqlite3(std::string_view&& file_name);
		~Sqlite3();

		auto exec(
		    std::string_view&& sql,
		    sqlite3_callback callback = nullptr,
		    void* obj = nullptr
		) -> void;

		auto status() const -> Status { return m_status; };

		auto message() const -> const char* { return m_err_msg; };

	   private:
		sqlite3* m_db;
		char* m_err_msg = nullptr;
		Status m_status = Status::OK;
	};

	auto status_str(Sqlite3::Status) -> std::string_view;
} // namespace sql
