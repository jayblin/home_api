#include "sql/json_string_result.hpp"

static bool is_numeric(const std::string_view& value)
{
	const auto it = std::find_if(
	    value.begin(),
	    value.end(),
	    [](char const& c) { return !std::isdigit(c); }
	);

	return it == value.end();
}

int JsonStringResult::callback(
    void* obj,
    int argc,
    char** argv,
    char** col_name
)
{
	int i;
	const auto stream = &static_cast<JsonStringResult*>(obj)->m_stream;

	*stream << "{";

	for (i = 0; i < argc; i++)
	{
		*stream << '"' << col_name[i] << "\":";

		if (argv[i])
		{
			const auto value = std::string_view {argv[i]};

			if (value.length() > 0 && is_numeric(value))
			{
				*stream << value;
			}
			else
			{
				*stream << '\"' << value << '\"';
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

	*stream << "},";

	return 0;
}

std::string JsonStringResult::get_array_result()
{
	m_stream << "]";
	std::stringstream tmp;
	tmp << "[";

	tmp << m_stream.rdbuf();
	m_stream = std::move(tmp);

	return m_stream.str();
}
