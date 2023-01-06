#ifndef LIB_SQL_FORWARD_H_
#define LIB_SQL_FORWARD_H_

#include "sqlite3.h"

namespace sql
{
	enum class Type
	{
		SQL_BLOB = SQLITE_BLOB,
		SQL_DOUBLE = SQLITE_FLOAT,
		SQL_INT = SQLITE_INTEGER,
		SQL_NULL = SQLITE_NULL,
		SQL_TEXT = SQLITE_TEXT,
	};

	class Statement;

	namespace status
	{
		/**
		 * @see https://www.sqlite.org/rescode.html
		 */
		enum class Code
		{
			ABORT = 4,
			AUTH = 23,
			BUSY = 5,
			CANTOPEN = 14,
			CONSTRAINT = 19,
			CORRUPT = 11,
			DONE = 101,
			EMPTY = 16,
			ERROR = 1,
			FORMAT = 24,
			FULL = 13,
			INTERNAL = 2,
			INTERRUPT = 9,
			IOERR = 10,
			LOCKED = 6,
			MISMATCH = 20,
			MISUSE = 21,
			NOLFS = 22,
			NOMEM = 7,
			NOTADB = 26,
			NOTFOUND = 12,
			NOTICE = 27,
			OK = 0,
			PERM = 3,
			PROTOCOL = 15,
			RANGE = 25,
			READONLY = 8,
			ROW = 100,
			SCHEMA = 17,
			TOOBIG = 18,
			WARNING = 28,
		};
	}
}

#endif // LIB_SQL_FORWARD_H_
