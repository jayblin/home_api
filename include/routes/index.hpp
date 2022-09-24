#include "http/request.hpp"
#include "http/response.hpp"

namespace routes
{
	auto index(http::Request&) -> http::Response;
} // namespace routes
