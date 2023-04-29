#include "util/http.hpp"

http::Response
    util::http::generate_db_error_response(const sqlw::Connection& con)
{
	return ::http::Response {}
	    .code(::http::Code::INTERNAL_SERVER_ERROR)
			.content("placeholder")
	    /* .content(util::json::create_errors_json( */
	    /*     {"Database coonection error", sqlw::status::verbose(con.status())} */
	    /* )) */
	    .content_type(::http::ContentType::APP_JSON);
}

http::Response
    util::http::generate_stmt_error_response(const sqlw::Statement& stmt)
{
	return ::http::Response {}
	    .code(::http::Code::INTERNAL_SERVER_ERROR)
			.content("placeholder")
	    /* .content(util::json::create_errors_json( */
	    /*     {"SQL-statement error", sqlw::status::verbose(stmt.status())} */
	    /* )) */
	    .content_type(::http::ContentType::APP_JSON);
}

bool util::http::fallback(std::optional<::http::Response>& response)
{
	response.emplace(::http::Response {}.content("fallback"));

	return true;
}
