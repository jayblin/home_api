#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "utils.hpp"

#include <string>

namespace http
{
	struct Response
	{
		const http::Code code = http::Code::OK;
		const std::string content = "";
		const http::ContentType content_type = http::ContentType::TEXT_HTML;
		const http::Charset charset = http::Charset::UTF_8;
	};
}

#endif // RESPONSE_H_
