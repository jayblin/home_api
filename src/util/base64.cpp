#include "util/base64.hpp"
#include <string>

constexpr std::string_view b64_table {
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

std::string decode_base64(std::string_view str)
{
	char result[str.length() * 6 / 8];
	size_t k = 0;

	for (size_t i = 0; i < str.length(); i += 4)
	{
		const auto a = str[i];
		const auto b = str[i + 1];
		const auto c = str[i + 2];
		const auto d = str[i + 3];

		int ca = 0;
		int cb = 0;
		int cc = 0;
		int cd = 0;

		for (size_t j = 0; j < b64_table.length(); j++)
		{
			if (b64_table[j] == a)
			{
				ca = j;
			}
			if (b64_table[j] == b)
			{
				cb = j;
			}
			if (b64_table[j] == c)
			{
				cc = j;
			}
			if (b64_table[j] == d)
			{
				cd = j;
			}
		}

		result[k++] = (ca << 2) + (cb >> 4);
		result[k++] = (cb << 4) + (cc >> 2);
		result[k++] = (cc << 6) + cd;
	}

	return result;
}
