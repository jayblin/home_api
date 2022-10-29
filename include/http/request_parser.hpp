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
		auto parse(http::Request&, http::Parsor&) -> void;
		auto is_finished() const -> bool;
	private:
		http::StatusLineParser m_slp;
		http::HeadersParser m_hp;
		std::stringstream m_stream;
	};

} // namespace http

#endif // REQUEST_PARSER_H_
