#include "sql/json_string_result.hpp"
#include "sql/forward.hpp"

static bool is_numeric(const std::string_view& value)
{
	const auto it = std::find_if(
	    value.begin(),
	    value.end(),
	    [](char const& c) { return !std::isdigit(c) && c != '.'; }
	);

	return it == value.end();
}

static bool is_json_array(const std::string_view& value)
{
	return value[0] == '[' && value[value.length() - 1] == ']';
}

static bool should_be_quoted(const std::string_view& value)
{
	return !(value.length() > 0
		&& (is_numeric(value) || is_json_array(value)))
	;
}

int sql::JsonStringResult::callback(
    void* obj,
    int argc,
    char** argv,
    char** col_name
)
{
	int i;
	const auto stream = &static_cast<JsonStringResult*>(obj)->m_stream;

	if (stream->rdbuf()->in_avail() > 0)
	{
		*stream << "},{";
	}
	else
	{
		*stream << "{";
	}

	for (i = 0; i < argc; i++)
	{
		*stream << '"' << col_name[i] << "\":";

		if (argv[i])
		{
			const auto value = std::string_view {argv[i]};

			if (should_be_quoted(value))
			{
				*stream << '\"' << value << '\"';
			}
			else
			{
				*stream << value;
			}

			if (i + 1 < argc)
			{
				*stream << ',';
			}
		}
		else
		{
			*stream << "null";
		}
	}

	return 0;
}

void sql::JsonStringResult::close_braces_if_needed()
{
	if (m_stream.view().starts_with('{') && !m_stream.view().ends_with('}'))
	{
		m_stream << "}";
	}
}

std::string sql::JsonStringResult::get_array_result()
{
	close_braces_if_needed();

	m_stream << "]";
	std::stringstream tmp;
	tmp << "[";

	tmp << m_stream.rdbuf();
	m_stream = std::move(tmp);

	return m_stream.str();
}

std::string sql::JsonStringResult::get_object_result()
{
	close_braces_if_needed();

	return m_stream.str();
}

void sql::JsonStringResult::row(int column_count)
{
	if (m_stream.rdbuf()->in_avail() > 0)
	{
		m_stream << "},{";
	}
	else
	{
		m_stream << "{";
	}
}

void sql::JsonStringResult::column(
	std::string_view name, sql::Type type, std::string_view value
)
{
	if (!m_stream.view().ends_with('{'))
	{
		m_stream << ',';
	}

	m_stream << '"' << name << "\":";

	if (sql::Type::SQL_NULL == type)
	{
		m_stream << "null";
	}
	else if (should_be_quoted(value))
	{
		m_stream << '\"' << value << '\"';
	}
	else
	{
		m_stream << value;
	}
}
