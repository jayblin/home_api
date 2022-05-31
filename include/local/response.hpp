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
	const http::Code code;
	const std::string content;
	const std::string content_type;
};

#endif // RESPONSE_H_
