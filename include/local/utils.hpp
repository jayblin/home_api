#include <iostream>

#ifndef UTILS_H_
#define UTILS_H_

#define CLOG(x) std::cout << x << "\n"

namespace http
{

enum class Method
{
	GET,
};

enum class Code
{
	OK = 200,
	NotFound = 404,
};

constexpr std::string_view method_to_str(const Method method)
{
	switch (method)
	{
		case http::Method::GET:
			return "GET";
	}
}

constexpr std::string_view code_to_str(const Code code)
{
	switch (code) {
		case http::Code::OK:
			return "OK";
		case http::Code::NotFound:
			return "Not Found";
	}
}

}

#endif // UTILS_H_
