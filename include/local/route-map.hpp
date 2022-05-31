#include "utils.hpp"
#include "request.hpp"
#include "response.hpp"

#include <string>

struct RouteInitializer
{
	http::Method method;
	std::string route;
	Response (*callback)(Request&);
};

class RouteMap
{
public:
	RouteMap(std::initializer_list<RouteInitializer>);

	auto match_method_with_request(Request&) -> Response;

private:
	std::unordered_map<std::string, Response (*)(Request&)> m_map;
};
