#ifndef HTTP_QUERY_PARSER_H_
#define HTTP_QUERY_PARSER_H_

#include "http/parsor.hpp"
#include "http/request.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace http
{
	struct QueryValue
	{
		std::string_view value {""};
		std::vector<std::string_view> values {};
	};

	class QueryParser
	{
	public:
		enum class State
		{
			START,
			VAR_NAME,
			VAR_VALUE,
			FINISHED,
		};

		auto parse(Parsor&)
		    -> std::unordered_map<std::string_view, QueryValue>;

		auto is_finished() const -> bool
		{
			return State::FINISHED == m_state;
		}

	private:
		State m_state = State::START;
		size_t m_var_name_end = 0;
		size_t m_var_value_end = -1;
	};

	namespace internal
	{
		class SequenceParser
		{
		public:
			auto parse(Parsor&, char delimiter) -> std::vector<std::string>;
		};
	} // namespace internal
} // namespace http

#endif // HTTP_QUERY_PARSER_H_
