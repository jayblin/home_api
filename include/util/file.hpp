#ifndef UTIL_FILE_H_
#define UTIL_FILE_H_

#include <string_view>

namespace util::file
{
	auto check_file_and_write_on_empty(
	    const std::string_view file_path,
	    const std::string_view stub
	) -> void;
}

#endif // UTIL_FILE_H_
