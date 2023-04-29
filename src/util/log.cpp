#include "util/log.hpp"

void util::log::log_as_hex(const char* buff, const size_t n)
{
	for (size_t i = 0; i < n; i++)
	{
		if (buff[i] == '\r' || buff[i] == '\n')
		{
			std::cout << YELLOW("0x0" << std::hex << (int) buff[i]) << std::dec
			          << " ";
		}
		else
		{
			std::cout << "0x0" << std::hex << (int) buff[i] << std::dec << " ";
		}
	}

	std::cout << '\n';
}
