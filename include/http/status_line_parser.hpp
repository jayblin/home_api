#ifndef HTTP_REQUEST_LEINE_PARSER_H_
#define HTTP_REQUEST_LEINE_PARSER_H_

#include "http/parsor.hpp"
#include "http/request.hpp"

namespace http
{
	class StatusLineParser
	{
	public:
		enum class State
		{
			START,
			METHOD_END,
			PATH_END,
			QUERY,
			QUERY_END,
			/* HTTP_VERSION_END, */
			FINISHED,
		};

		auto parse(Request&, Parsor&) -> State;

		auto is_finished() const -> bool
		{
			return m_state == State::FINISHED;
		}

		auto reset() -> void
		{
			m_state = State::START;
			m_method_end = 0;
			m_path_end = 0;
			m_query_end = 0;
		}

	private:
		State m_state = State::START;
		size_t m_method_end = 0;
		size_t m_path_end = 0;
		size_t m_query_end = 0;
	};
} // namespace http

#endif // HTTP_REQUEST_LEINE_PARSER_H_
