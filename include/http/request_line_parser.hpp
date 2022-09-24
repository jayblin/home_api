#ifndef HTTP_REQUEST_LEINE_PARSER_H_
#define HTTP_REQUEST_LEINE_PARSER_H_

#include <string>
#include <unordered_map>

namespace http
{
	class RequestLineParser
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

		auto parse(const char* buffer, const size_t cur_pos) -> State;

		auto is_finished() const -> bool { return m_state == State::FINISHED; }

		std::string method;
		std::string path;
		/* std::unordered_map<std::string, std::string> query; */
		std::string query;
		std::string http_version;

	private:
		State m_state = State::START;
		size_t m_method_end = 0;
		size_t m_path_end = 0;
		size_t m_query_end = 0;
	};
} // namespace http

#endif // HTTP_REQUEST_LEINE_PARSER_H_
