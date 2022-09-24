#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include "http/headers.hpp"
#include "utils.hpp"
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
} // namespace http

#endif // HTTP_REQUEST_H_
