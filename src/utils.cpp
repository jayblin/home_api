#include "utils.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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

auto json_doc_to_string(rapidjson::Document& doc) -> std::string
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	doc.Accept(writer);
	
	return buffer.GetString();
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
