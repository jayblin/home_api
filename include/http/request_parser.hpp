#ifndef REQUEST_PARSER_H_
#define REQUEST_PARSER_H_

#include "http/parsor.hpp"
#include "request.hpp"
#include "http/headers_parser.hpp"
#include "http/status_line_parser.hpp"

namespace http
{
	class RequestParser
	{
	public:
		RequestParser(http::Request& request): m_request(request) {};
		auto parse(http::Parsor&) -> void;
		auto is_finished() const -> bool;
	private:
		http::Request& m_request;
		http::StatusLineParser m_slp{};
		http::HeadersParser m_hp{};
		std::stringstream m_stream{};
	};

} // namespace http

#endif // REQUEST_PARSER_H_
