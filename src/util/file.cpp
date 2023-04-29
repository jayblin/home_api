#include "util/file.hpp"
#include <filesystem>
#include <fstream>

auto util::file::check_file_and_write_on_empty(
    const std::string_view file_path,
    const std::string_view stub
) -> void
{
	if (!std::filesystem::exists(file_path))
	{
		auto file = std::ofstream {file_path.data()};
		file << stub;
		file.close();
	}
}
