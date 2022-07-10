#ifndef HTTP_HEADER_PARSER_H_
#define HTTP_HEADER_PARSER_H_

#include <string>

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

		auto parse(const char* buffer, const size_t cur_pos) -> State;
		auto is_finished() const -> bool { return m_state == State::END; }

		std::string host;
		size_t 		content_length = 0;

	private:
		State 		m_state = State::START;
		size_t 		m_key_start = -1;
		size_t		m_key_len = -1;
		size_t		m_value_start = -1;
		size_t		m_value_len = -1;
	};
}

#endif // HTTP_HEADER_PARSER_H_
