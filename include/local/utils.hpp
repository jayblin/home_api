#include <iostream>

#define CLOG(x) std::cout << x << "\n"

enum class HttpMethod
{
	GET,
};

enum class HttpCode
{
	OK = 200,
	NotFound = 404,
};
