#include "http/status_line_parser.hpp"

http::StatusLineParser::State
    http::StatusLineParser::parse(http::Request& request, http::Parsor& parsor)
{
	const auto view = parsor.view();
	auto iters = 0;

	while (!is_finished() && iters < 1'000)
	{
		iters++;
		const auto ch = parsor.cur_char();

		if (m_state == State::START)
		{
			if (ch == ' ')
			{
				request.method = view.substr(0, parsor.cur_pos());

				m_method_end = parsor.cur_pos();
				m_state = State::METHOD_END;
			}
		}
		else if (m_state == State::METHOD_END)
		{
			if (ch == ' ' || ch == '?')
			{
				request.target = view.substr(
				    m_method_end + 1,
				    parsor.cur_pos() - m_method_end - 1
				);

				m_path_end = parsor.cur_pos();

				if (ch == '?')
				{
					m_state = State::QUERY;
				}
				else
				{
					m_query_end = parsor.cur_pos();
					m_state = State::PATH_END;
				}
			}
		}
		else if (m_state == State::QUERY)
		{
			if (ch == ' ')
			{
				request.query = view.substr(
				    m_path_end + 1,
				    parsor.cur_pos() - m_path_end - 1
				);

				this->m_query_end = parsor.cur_pos();

				m_state = State::QUERY_END;
			}
		}
		else if (m_state == State::PATH_END || m_state == State::QUERY_END)
		{
			if (ch == '\n' && view[parsor.cur_pos() - 1] == '\r')
			{
				// 6 == " HTTP/".length()
				const auto version = view.substr(
				    m_query_end + 6,
				    (view.length() - 2) - (m_query_end + 6)
				);

				request.http_version = version[0] - '0';

				m_state = State::FINISHED;
			}
		}

		parsor.advance();
	}

	return m_state;
}
