#include <string_view>
#include <string>
#include <unordered_map>
#include <vector>
#include "utils.hpp"
#include "headers.hpp"

#ifndef REQUEST_H_
#define REQUEST_H_

namespace http
{
	struct Request
	{
		std::string method;
		std::string path;
		std::string query;
		std::string body;
		http::Headers headers;
	};
}

#endif // REQUEST_H_

