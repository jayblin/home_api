#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "http/forward.hpp"
#include "http/headers.hpp"
#include <cstring>
#include <string>
#include <string_view>
/* #include <utility> */

namespace http
{
	struct Response
	{
	public:
		auto code(http::Code value) -> Response&
		{
			m_code = value;
			return *this;
		}

		auto content_type(http::ContentType value) -> Response&
		{
			m_content_type = value;
			return *this;
		}

		auto charset(http::Charset value) -> Response&
		{
			m_charset = value;
			return *this;
		}

		auto content(std::string&& value) -> Response&
		{
			m_content = value;
			return *this;
		}

		auto headers(std::string&& value) -> Response&
		{
			m_headers = value;
			return *this;
		}

		auto code() const -> http::Code { return m_code; };

		auto content_type() const -> http::ContentType
		{
			return m_content_type;
		};

		auto charset() const -> http::Charset { return m_charset; };

		auto content() const -> const std::string& { return m_content; };

		auto headers() const -> const std::string& { return m_headers; };

	private:
		http::Code m_code = http::Code::OK;
		http::ContentType m_content_type = http::ContentType::TEXT_HTML;
		http::Charset m_charset = http::Charset::UTF_8;
		std::string m_headers;
		std::string m_content;
	};
} // namespace http

#endif // RESPONSE_H_
