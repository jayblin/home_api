#include <iostream>
#include <vector>
#include <concepts>

#ifndef LOCAL_UTILS_H_
#define LOCAL_UTILS_H_

#define CLOG(x) std::cout << x << "\n"

template <class T>
concept provides_errors = requires(const T t)
{
	{ t.errors() } -> std::same_as<const std::vector<std::string>&>;
};

enum class HttpMethod
{
	GET,
};

enum class HttpCode
{
	OK = 200,
	NotFound = 404,
};

#endif // LOCAL_UTILS_H_
