#include "http/headers_parser.hpp"

http::HeadersParser::State http::HeadersParser::parse(
	const char* buffer,
	const size_t cur_pos
)
{
	if (m_key_start == -1)
	{
		m_key_start = cur_pos;
		m_state = State::KEY;
	}

	if (m_state == State::KEY)
	{
		if (buffer[cur_pos] == ':')
		{
			m_key_len = cur_pos - m_key_start;

			m_state = State::KEY_END;
		}
	}
	else if (m_state == State::KEY_END && buffer[cur_pos] != ' ')
	{
		m_state = State::VALUE;
		m_value_start = cur_pos;
	}
	else if (m_state == State::VALUE)
	{
		if (buffer[cur_pos - 1] == '\r' && buffer[cur_pos] == '\n')
		{
			m_value_len = cur_pos - m_value_start - 1;
			m_state = State::VALUE_END;
		}
	}
	else if (m_state == State::VALUE_END)
	{
		const auto key = std::string(
			buffer + m_key_start,
			m_key_len
		);

		const auto value = std::string(
			buffer + m_value_start,
			m_value_len
		);

		if (key.compare("Host") == 0)
		{
			this->host = std::move(value);
		}
		else if (key.compare("Content-Length") == 0)
		{
			this->content_length = std::stoll(value);
		}

		m_state = State::KEY;
		m_key_start = cur_pos;
	}

	if (buffer[cur_pos-1] == '\r' && buffer[cur_pos] == '\n'
		&& buffer[cur_pos-3] == '\r' && buffer[cur_pos - 2] == '\n'
	)
	{
		m_state = State::END;
	}

	return m_state;
}
