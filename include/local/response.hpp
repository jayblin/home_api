#include "utils.hpp"

#include <algorithm>
#include <string>

/**
 * Aggregate initialization.
 */
struct Response
{
	HttpCode code;
	std::string content;
	std::string content_type;
};

namespace response
{
	auto status(const Response&) -> std::string;
}

