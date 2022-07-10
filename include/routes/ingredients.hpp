#include "request.hpp"
#include "response.hpp"

#include <string>

namespace routes::ingredients
{

	struct Ingredient
	{
		int id;
		std::string title;
	};

	auto list(http::Request& request) -> Response;
	auto post(http::Request& request) -> Response;
}

