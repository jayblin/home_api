#include <iostream>

#ifndef UTILS_H_
#define UTILS_H_

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


#endif // UTILS_H_
