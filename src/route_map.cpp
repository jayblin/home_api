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
	const auto& path = request.target;
	std::size_t _i = 0;
	std::size_t i = 0;

	while (i != std::string::npos && i < path.size() && _i < 100)
	{
		_i++;
		i = path.find("/", i + 1);

		const auto route = path.substr(0, i);

		const auto key = std::string {request.method} + std::string {route};

		if (m_map.contains(key))
		{
			return m_map.at(key)(request, get_body);
		}
	}

	return http::Response {}.code(http::Code::NOT_FOUND);
}
