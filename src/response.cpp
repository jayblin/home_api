#include "response.hpp"
#include <algorithm>

std::string response::status(const Response& response)
{
	switch (response.code) {
		case HttpCode::OK:
			return "OK";
		case HttpCode::NotFound:
			return "Not Found";
		default:
			return "Code Not Implemented";
	}
}
