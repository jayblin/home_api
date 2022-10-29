#include "http/request_parser.hpp"
#include <sstream>
#include <string>
#include <algorithm>

void http::RequestParser::parse(
	http::Request& request,
	http::Parsor& parsor
)
{
	size_t iters = 0;
	bool is_finished = false;

	m_slp.parse(request, parsor);
	m_hp.parse(request.headers, parsor);

	if (
		request.headers.content_length > 0
		&& m_stream.view().length() < request.headers.content_length
		&& !parsor.is_end()
	)
	{
		const auto d = parsor.view().length() - parsor.cur_pos();
		const auto len = request.headers.content_length > d ?
			d :
			request.headers.content_length
		;

		m_stream << std::string(
			parsor.view().data(),
			parsor.cur_pos(),
			len
		);

		parsor.advance(len);
	}

	if (
		request.headers.content_length > 0
		&& m_stream.view().length() == request.headers.content_length
	)
	{
		request.body = std::move(m_stream.str());
	}
}
