#ifndef LOCAL_ROUTE_MAP_H_
#define LOCAL_ROUTE_MAP_H_

#include "utils.hpp"
#include "http/request.hpp"
#include "http/response.hpp"

#include <string>

struct RouteInitializer
{
	http::Method method;
	std::string route;
	http::Response (*callback)(http::Request&);
};

class RouteMap
{
public:
	RouteMap(std::initializer_list<RouteInitializer>);

	auto match_method_with_request(http::Request&) -> http::Response;

private:
	std::unordered_map<std::string, http::Response (*)(http::Request&)> m_map;
};

#endif // LOCAL_ROUTE_MAP_H_
