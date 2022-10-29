#include "http/request_parser.hpp"
#include <sstream>
#include <string>
#include <algorithm>

void http::RequestParser::parse(http::Parsor& parsor)
{
	size_t iters = 0;
	bool is_finished = false;

	m_slp.parse(m_request, parsor);
	m_hp.parse(m_request.headers, parsor);

	if (
		m_request.headers.content_length > 0
		&& m_stream.view().length() < m_request.headers.content_length
		&& !parsor.is_end()
	)
	{
		const auto d = parsor.view().length() - parsor.cur_pos();
		const auto len = m_request.headers.content_length > d ?
			d :
			m_request.headers.content_length
		;

		m_stream << std::string(
			parsor.view().data(),
			parsor.cur_pos(),
			len
		);

		parsor.advance(len);
	}

	if (
		m_request.headers.content_length > 0
		&& m_stream.view().length() == m_request.headers.content_length
	)
	{
		m_request.body = std::move(m_stream.str());
	}
}

bool http::RequestParser::is_finished() const
{
	return m_request.body.length() == m_request.headers.content_length;
}
