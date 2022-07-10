#include "routes/index.hpp"

#include "utils.hpp"

auto routes::index(http::Request&) -> Response
{
	return {
		http::Code::OK,
		"<!DOCTYPE html>"
		"<html>"
		"<body>"
		"INDEX PAGE"
		"</body>"
		"</html>"
	};
}

