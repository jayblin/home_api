#include "http/headers_parser.hpp"
#include "http/headers.hpp"
#include <charconv>

http::HeadersParser::State
    http::HeadersParser::parse(http::Headers& headers, http::Parsor& parsor)
{
	auto iters = 0;

	while (!is_finished() && iters < 1'000'000)
	{
		iters++;

		const auto ch = parsor.cur_char();
		const auto cur_pos = parsor.cur_pos();
		const auto& view = parsor.view();

		if (m_key_start == -1)
		{
			m_key_start = cur_pos;
			m_state = State::KEY;
		}

		if (m_state == State::KEY)
		{
			if (ch == ':')
			{
				m_key_len = cur_pos - m_key_start;

				m_state = State::KEY_END;
			}
		}
		else if (m_state == State::KEY_END && ch != ' ')
		{
			m_state = State::VALUE;
			m_value_start = cur_pos;
		}
		else if (m_state == State::VALUE)
		{
			if (ch == '\n' && view[parsor.cur_pos() - 1] == '\r')
			{
				m_value_len = cur_pos - m_value_start - 1;
				m_state = State::VALUE_END;
			}
		}
		else if (m_state == State::VALUE_END)
		{
			const auto key = view.substr(m_key_start, m_key_len);
			const auto value = view.substr(m_value_start, m_value_len);

			if (key.compare("Host") == 0)
			{
				headers.host = std::move(value);
			}
			else if (key.compare("Content-Length") == 0)
			{
				// @todo Add error handling @see from_chars struct.
				const auto converted = std::from_chars(
					value.data(),
					value.data() + value.length(),
					headers.content_length
				);
			}

			m_state = State::KEY;
			m_key_start = cur_pos;
		}

		if (view[cur_pos - 1] == '\r' && view[cur_pos] == '\n' &&
			view[cur_pos - 3] == '\r' && view[cur_pos - 2] == '\n')
		{
			m_state = State::END;
		}

		parsor.advance();
	}

	return m_state;
}
