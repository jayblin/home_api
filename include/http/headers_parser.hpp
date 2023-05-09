#ifndef HTTP_HEADER_PARSER_H_
#define HTTP_HEADER_PARSER_H_

#include "http/headers.hpp"
#include "http/parsor.hpp"

namespace http
{
	class HeadersParser
	{
	public:
		enum class State
		{
			START,
			KEY,
			KEY_END,
			VALUE,
			VALUE_END,
			END
		};

		auto parse(http::Headers&, http::Parsor&) -> State;

		auto is_finished() const -> bool
		{
			return m_state == State::END;
		}

		auto reset() -> void
		{
			m_state = State::START;
			m_key_start = -1;
			m_key_len = -1;
			m_value_start = -1;
			m_value_len = -1;
		}

	private:
		State m_state = State::START;
		size_t m_key_start = -1;
		size_t m_key_len = -1;
		size_t m_value_start = -1;
		size_t m_value_len = -1;
	};
} // namespace http

#endif // HTTP_HEADER_PARSER_H_
