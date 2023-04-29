#ifndef UTIL_LOG_H_
#define UTIL_LOG_H_

#include <iostream>

#define CLOG(x) std::cout << __FILE__ << ":" << __LINE__ << " " << x << "\n"

#define YELLOW(x) "\x1B[33m" << x << "\033[0m"

namespace util::log
{
	auto log_as_hex(const char* buff, const size_t n) -> void;
}

#endif // UTIL_LOG_H_
