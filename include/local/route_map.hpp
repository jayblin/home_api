#ifndef LOCAL_ROUTE_MAP_H_
#define LOCAL_ROUTE_MAP_H_

#include "http/request.hpp"
#include "http/response.hpp"
#include <functional>
#include <string>
#include <string_view>
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
	using BodyGetter = std::function<std::string()>;
	using Route =
	    std::function<http::Response(http::Request&, BodyGetter& get_body)>;

	static RouteMap& instance()
	{
		static RouteMap _instance;

		return _instance;
	}

	/**
	 * Based on request executes a function and returns a response of
	 * that function.
	 */
	auto match_method_with_request(
	    http::Request&,
	    std::function<std::string()>& get_body
	) const -> http::Response;
	auto
	    add(http::Method method,
	        std::string_view path,
	        std::function<http::Response(http::Request&, BodyGetter& get_body)>
	            callback) -> RouteMap&;

private:
	RouteMap();
	RouteMap(const RouteMap&);
	RouteMap& operator=(const RouteMap&);

	std::unordered_map<std::string, Route> m_map;
};

#endif // LOCAL_ROUTE_MAP_H_
