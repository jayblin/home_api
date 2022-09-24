#include "route_map.hpp"
#include "utils.hpp"
#include <sstream>

RouteMap::RouteMap(std::initializer_list<RouteInitializer> list)
{
	for (auto& item : list)
	{
		auto ss = std::stringstream {};
		ss << http::method_to_str(item.method) << item.route;

		m_map.insert_or_assign(ss.str(), item.callback);
	}
}

http::Response RouteMap::match_method_with_request(http::Request& request)
{
	const auto& path = request.path;
	std::size_t _i = 0;
	std::size_t i = 0;

	while (i != std::string::npos && i < path.size() && _i < 100)
	{
		_i++;
		i = path.find("/", i + 1);

		const auto route = path.substr(0, i);

		const auto key = request.method + route;
		const auto func = m_map[request.method + route];

		if (func)
		{
			return func(request);
		}
	}

	return http::Response {}.code(http::Code::NOT_FOUND);
}
