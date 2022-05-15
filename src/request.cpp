#include "request.hpp"

#include <sstream>

Request::Request(const std::string_view raw_request)
{
	auto ss = std::istringstream{raw_request.data()};
	std::string line;

	auto i = 0;
	while (std::getline(ss, line))
	{
		++i;

		if (i == 1)
		{
			const auto method_end = line.find(" ", 0);
			
			this->method = line.substr(0, method_end);

			const auto path_end = line.find(" ", method_end + 1);
			
			const auto raw_path = line.substr(
				method_end + 1,
				path_end - method_end - 1
			);
			const auto query_start = raw_path.find("?");

			this->query = raw_path.substr(query_start + 1);
			this->path = raw_path.substr(0, query_start);

			continue;
		}

		const auto key_end = line.find(":");
		const auto key = line.substr(0, key_end);

		if (key.compare("Host") == 0)
		{
			this->host = line.substr(key_end + 2);
		}
	}
}
	
Request::~Request() {}
