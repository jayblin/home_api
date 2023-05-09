#ifndef UTIL_HTTP_H_
#define UTIL_HTTP_H_

#include "http/response.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include <optional>

namespace util::http
{
	bool fallback(std::optional<::http::Response>& response);

	::http::Response generate_db_error_response(const sqlw::Connection& con);

	::http::Response generate_stmt_error_response(const sqlw::Statement& stmt);

} // namespace util::http

#endif // UTIL_HTTP_H_
