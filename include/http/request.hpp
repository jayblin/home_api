#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include "utils.hpp"

#include "http/headers.hpp"

#include <string>

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

#endif // HTTP_REQUEST_H_
