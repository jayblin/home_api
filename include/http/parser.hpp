#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_

#include "http/parsor.hpp"
#include <array>
#include <concepts>
#include <type_traits>

namespace http
{
	namespace parser
	{
		// clang-format off
		template<typename T>
		concept is_context = requires(T t, size_t DelimSize)
		{
			{ T::names_size };
			requires std::same_as<decltype(T::names_size), const size_t>;

			{ t.start_str };
			requires std::same_as<decltype(t.start_str), const std::string_view>;

			{ t.declaration_delimiter };
			requires std::same_as<decltype(t.declaration_delimiter), const std::string_view>;

			{ t.value_delimiters };
			requires std::ranges::range<decltype(t.value_delimiters)>;

			{ t.names };
			requires std::ranges::range<decltype(t.names)>;
		};

		// clang-format on

		template<size_t NamesSize, size_t DelimSize>
		struct Context
		{
			static constexpr size_t names_size = NamesSize;

			const std::array<std::string_view, NamesSize> names;
			const std::array<std::string_view, DelimSize> value_delimiters;
			const std::string_view declaration_delimiter;
			const std::string_view start_str;
		};

		static_assert(is_context<Context<1,1>>);
	} // namespace parser

	class Parser
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

		template<typename T>
		requires parser::is_context<T>
		auto parse(http::Parsor& parsor, T ctx)
		    -> std::array<std::string_view, T::names_size>;

		auto is_finished() const -> bool
		{
			return m_state == State::END;
		}

		auto reset() -> void
		{
			m_state = State::START;
		}

	private:
		State m_state = State::START;
	};
} // namespace http

template<typename T>
requires http::parser::is_context<T>
auto http::Parser::parse(http::Parsor& parsor, T ctx)
    -> std::array<std::string_view, T::names_size>
{
	std::array<std::string_view, T::names_size> result;

	const auto npos = std::string_view::npos;
	const auto& view = parsor.view();

	std::string_view::size_type start_pos = 0;

	if (ctx.start_str.length() > 0)
	{
		start_pos = view.find(ctx.start_str);

		if (start_pos == npos)
		{
			m_state = Parser::State::END;
			return result;
		}
	}

	size_t i = 0;
	for (const std::string_view& name : ctx.names)
	{
		const auto entry_start =
		    view.find(name, start_pos + ctx.start_str.length());

		if (entry_start == npos)
		{
			continue;
		}

		const auto assignment_pos =
		    view.find(ctx.declaration_delimiter, entry_start);

		if (assignment_pos == npos)
		{
			continue;
		}

		auto entry_end = npos;

		for (const std::string_view& delim : ctx.value_delimiters)
		{
			entry_end = view.find(delim, assignment_pos);

			if (entry_end != npos)
			{
				break;
			}
		}

		if (entry_end == npos)
		{
			continue;
		}

		result[i++] =
		    view.substr(assignment_pos + 1, entry_end - assignment_pos - 1);
	}

	return result;
}

#endif // HTTP_PARSER_H_
