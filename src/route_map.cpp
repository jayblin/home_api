#include "local/route_map.hpp"
#include <sstream>

RouteMap& RouteMap::add(
	http::Method method,
	std::string route,
	std::function<http::Response (http::Request&)> callback
)
{
	auto ss = std::stringstream {};
	ss << http::method_to_str(method) << route;

	m_map.insert_or_assign(ss.str(), callback);

	return *this;
}

http::Response RouteMap::match_method_with_request(http::Request& request) const
{
	const auto& path = request.target;
	std::size_t _i = 0;
	std::size_t i = 0;

	while (i != std::string::npos && i < path.size() && _i < 100)
	{
		_i++;
		i = path.find("/", i + 1);

		const auto route = path.substr(0, i);

		const auto key = request.method + route;

		if (m_map.contains(request.method + route))
		{
			return m_map.at(request.method + route)(request);
		}
	}

	return http::Response {}.code(http::Code::NOT_FOUND);
}
