#ifndef LOCAL_ROUTE_MAP_H_
#define LOCAL_ROUTE_MAP_H_

#include "http/request.hpp"
#include "http/response.hpp"
#include <functional>
#include <string>
#include <unordered_map>

struct RouteInitializer
{
	http::Method method;
	std::string route;
	http::Response (*callback)(http::Request&);
};

class RouteMap
{
public:
	static RouteMap& instance()
	{
		static RouteMap _instance;

		return _instance;
	}

	/**
	 * Based on request executes a function and returns a response of
	 * that function.
	 */
	auto match_method_with_request(http::Request&) const -> http::Response;
	/* auto add(std::function<http::Response(http::Request&)>) -> RouteMap&; */
	auto
	    add(http::Method method,
	        std::string route,
	        std::function<http::Response(http::Request&)> callback)
	        -> RouteMap&;
	/* auto */
	/*     add(http::Method method, */
	/*         std::string route, */
	/*         std::function<http::Response(http::Request&)> callback) */
	/*         -> RouteMap&; */

private:
	RouteMap();
	RouteMap(const RouteMap&);
	RouteMap& operator=(const RouteMap&);

	std::unordered_map<std::string, std::function<http::Response(http::Request&)>>
	    m_map;
};

#endif // LOCAL_ROUTE_MAP_H_
