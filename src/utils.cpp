#include "utils.hpp"
#include <filesystem>
#include <fstream>

void log_as_hex(const char* buff, const size_t n)
{
	for (size_t i = 0; i < n; i++)
	{
		if (buff[i] == '\r' || buff[i] == '\n')
		{
			std::cout
				<< YELLOW(
					"0x0"
					<< std::hex
					<< (int) buff[i]
				)
				<< std::dec << " "
			;
		}
		else
		{
			std::cout
				<< "0x0"
				<< std::hex
				<< (int) buff[i]
				<< std::dec << " "
			;
		}
	}

	std::cout << '\n';
}

auto check_file_and_write_on_empty(
	const std::string_view file_path,
	const std::string_view stub
) -> void
{
	if (!std::filesystem::exists(file_path))
	{
		auto file = std::ofstream{file_path.data()};
		file << stub;
		file.close();
	}
}
