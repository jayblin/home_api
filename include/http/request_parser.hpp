#ifndef REQUEST_PARSER_H_
#define REQUEST_PARSER_H_

#include "http/parsor.hpp"
#include "http/request.hpp"
#include "http/headers_parser.hpp"
#include "http/status_line_parser.hpp"
#include <sstream>

namespace http
{
	class RequestParser
	{
	public:
		RequestParser(http::Request& request): m_request(request) {};
		auto parse(http::Parsor&) -> void;
		auto is_finished() const -> bool;
		auto is_request_line_finished() const -> bool;
		auto are_headers_finished() const -> bool;
		// ??? auto request() -> http::Request;
	private:
		http::Request& m_request;
		http::StatusLineParser m_slp{};
		http::HeadersParser m_hp{};
		std::stringstream m_stream{};
	};

} // namespace http

#endif // REQUEST_PARSER_H_
