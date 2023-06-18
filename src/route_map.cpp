#include "local/route_map.hpp"
#include <sstream>

RouteMap& RouteMap::add(
    http::Method method,
    std::string_view path,
    RouteMap::Route callback
)
{
	auto ss = std::stringstream {};
	ss << http::method_to_str(method) << path;

	m_map.insert_or_assign(ss.str(), callback);

	return *this;
}

http::Response RouteMap::match_method_with_request(
    http::Request& request,
    RouteMap::BodyGetter& get_body
) const
{
	std::string key =
	    std::string {request.method} + std::string {request.target};

	if (m_map.contains(key))
	{
		return m_map.at(key)(request, get_body);
	}

	for (size_t i = request.target.length() - 1; i > 0; i--)
	{
		if ('/' == request.target[i])
		{
			key = std::string {request.method}
			    + std::string {request.target.substr(0, i)};

			if (m_map.contains(key))
			{
				return m_map.at(key)(request, get_body);
			}
		}
	}

	return http::Response {}.code(http::Code::NOT_FOUND);
}
