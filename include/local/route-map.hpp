#ifndef HTTP_ROUTE_MAP_H_
#define HTTP_ROUTE_MAP_H_

#include "utils.hpp"
#include "request.hpp"
#include "response.hpp"

#include <string>

struct RouteInitializer
{
	http::Method method;
	std::string route;
	Response (*callback)(http::Request&);
};

class RouteMap
{
public:
	RouteMap(std::initializer_list<RouteInitializer>);

	auto match_method_with_request(http::Request&) -> Response;

private:
	std::unordered_map<std::string, Response (*)(http::Request&)> m_map;
};

#endif // HTTP_ROUTE_MAP_H_
