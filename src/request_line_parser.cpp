#include "http/request_line_parser.hpp"
#include "utils.hpp"

http::RequestLineParser::State
    http::RequestLineParser::parse(const char* buffer, const size_t cur_pos)
{
	// http://localhost:3000?id=12
	if (m_state == State::START)
	{
		if (buffer[cur_pos] == ' ')
		{
			this->method = std::string(buffer, cur_pos);

			m_method_end = cur_pos;
			m_state = State::METHOD_END;
		}
	}
	else if (m_state == State::METHOD_END)
	{
		if (buffer[cur_pos] == ' ' || buffer[cur_pos] == '?')
		{
			this->path = std::string(
			    buffer + m_method_end + 1,
			    cur_pos - m_method_end - 1
			);

			m_path_end = cur_pos;

			if (buffer[cur_pos] == '?')
			{
				m_state = State::QUERY;
			}
			else
			{
				m_query_end = cur_pos;
				m_state = State::PATH_END;
			}
		}
	}
	else if (m_state == State::QUERY)
	{
		if (buffer[cur_pos] == ' ')
		{
			this->query =
			    std::string(buffer + m_path_end + 1, cur_pos - m_path_end - 1);

			this->m_query_end = cur_pos;

			m_state = State::QUERY_END;
		}
	}
	else if (m_state == State::PATH_END || m_state == State::QUERY_END)
	{
		if (buffer[cur_pos - 1] == '\r' && buffer[cur_pos] == '\n')
		{
			this->http_version = std::string(
			    buffer + m_query_end + 1,
			    cur_pos - m_query_end - 2
			);

			m_state = State::FINISHED;
		}
	}

	return m_state;
}
