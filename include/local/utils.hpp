#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <sstream>

#include "rapidjson/document.h"

#define CLOG(x) std::cout << __FILE__ << ":" << __LINE__ << " " << x << "\n"

#define YELLOW(x) "\x1B[33m" << x << "\033[0m"

namespace http
{
	enum class Method
	{
		GET,
		POST,
	};

	enum class Code
	{
		OK = 200,
		NOT_FOUND = 404,
		BAD_REQUEST = 400,
		INTERNAL_SERVER_ERROR = 500,
	};

	enum class ContentType
	{
		APP_JSON,
		TEXT_HTML,
		IMG_PNG,
		IMG_JPG,
		IMG_X_ICON,
	};

	enum class Charset
	{
		UTF_8,
	};

	constexpr std::string_view method_to_str(const Method method)
	{
		switch (method)
		{
			case http::Method::GET:
				return "GET";
			case http::Method::POST:
				return "POST";
		}
	}

	constexpr std::string_view code_to_str(const Code code)
	{
		switch (code)
		{
			case http::Code::OK:
				return "OK";
			case http::Code::NOT_FOUND:
				return "Not Found";
			case Code::BAD_REQUEST:
				return "Bad Request";
			case Code::INTERNAL_SERVER_ERROR:
				return "Internal Server Error";
		}
	}

	constexpr int code_to_int(const Code code)
	{
		return static_cast<int>(code);
	}

	constexpr std::string_view content_type_to_str(const ContentType ct)
	{
		switch (ct)
		{
			case ContentType::APP_JSON:
				return "application/json";
			case ContentType::TEXT_HTML:
				return "text/html";
			case ContentType::IMG_PNG:
				return "image/png";
			case ContentType::IMG_JPG:
				return "image/jpg";
			case ContentType::IMG_X_ICON:
				return "image/x-icon";
		}
	}

	constexpr std::string_view charset_to_str(const Charset ch)
	{
		switch (ch)
		{
			case Charset::UTF_8:
				return "utf-8";
		}
	}
}

auto log_as_hex(const char* buff, const size_t n) -> void;

auto json_doc_to_string(rapidjson::Document& doc) -> std::string;

auto check_file_and_write_on_empty(
	const std::string_view file_path,
	const std::string_view stub
) -> void;

#endif // UTILS_H_
