#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include "http/forward.hpp"
#include "http/headers.hpp"
#include <string>
#include <string_view>

namespace http
{
	struct Request
	{
		std::string raw;
		std::string_view method;
		std::string_view target;
		std::string_view query;
		int http_version;
		/* std::string_view body; */
		http::Headers headers;

		auto size() -> size_t; //!!!!!!!!!!!!!
	};
} // namespace http

#endif // HTTP_REQUEST_H_
