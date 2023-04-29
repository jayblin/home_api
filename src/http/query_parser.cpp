#include "http/query_parser.hpp"
#include <iostream>

std::vector<std::string>
    http::internal::SequenceParser::parse(http::Parsor& parsor, char delimiter)
{
	std::vector<std::string> result;
	bool is_finished = false;
	size_t iters = 0;
	const auto view = parsor.view();
	size_t last_val_end = 0;

	while (!is_finished && iters < 1'000)
	{
		const auto ch = parsor.cur_char();

		if (ch == delimiter)
		{
			result.emplace_back(
			    view.substr(last_val_end, parsor.cur_pos() - last_val_end)
			);
			last_val_end = parsor.cur_pos() + 1;
		}

		if (parsor.is_end())
		{
			result.emplace_back(
			    view.substr(last_val_end, parsor.cur_pos() - last_val_end + 1)
			);
			is_finished = true;
		}

		parsor.advance();
	}

	return result;
}

std::unordered_map<std::string_view, http::QueryValue>
    http::QueryParser::parse(http::Parsor& parsor)
{
	std::unordered_map<std::string_view, http::QueryValue> result;
	size_t iters = 0;
	std::string_view var_name;
	const auto view = parsor.view();

	while (!is_finished() && iters < 1'000)
	{
		iters++;

		const auto ch = parsor.cur_char();
		const auto pos = parsor.cur_pos();

		if (State::START == m_state)
		{
			m_state = State::VAR_NAME;
		}
		else if (State::VAR_NAME == m_state)
		{
			if (ch == '=')
			{
				m_var_name_end = pos;

				var_name = view.substr(
				    m_var_value_end + 1,
				    m_var_name_end - m_var_value_end - 1
				);

				m_state = State::VAR_VALUE;
			}
		}
		else if (State::VAR_VALUE == m_state)
		{
			if (ch == '&' || parsor.is_end())
			{
				m_var_value_end = pos;
				const auto rcount = m_var_value_end - m_var_name_end
				                  - (parsor.is_end() ? 0 : 1);

				if (result.contains(var_name))
				{
					auto& item = result[var_name];

					if (item.values.size() == 0)
					{
						item.values.push_back(item.value);
					}

					item.values.push_back(
					    view.substr(m_var_name_end + 1, rcount)
					);
				}
				else
				{
					result.insert_or_assign(
					    var_name,
					    QueryValue {
					        .value = view.substr(m_var_name_end + 1, rcount)}
					);
				}

				m_state = State::VAR_NAME;
			}
		}

		if (parsor.is_end())
		{
			m_state = State::FINISHED;
		}

		parsor.advance();
	}

	return result;
}
