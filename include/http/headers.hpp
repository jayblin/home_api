#ifndef HTTP_HEADERS_H_
#define HTTP_HEADERS_H_

#include <string>

namespace http
{
	struct Headers
	{
		size_t content_length = 0;
		std::string host = "";
	};
} // namespace http

#endif // HTTP_HEADERS_H_
