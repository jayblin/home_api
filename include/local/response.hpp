#include "utils.hpp"

#include <algorithm>
#include <string>

#ifndef RESPONSE_H_
#define RESPONSE_H_

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


#endif // RESPONSE_H_
