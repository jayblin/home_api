#include "routes/index.hpp"
#include "utils.hpp"

using R = http::Response;

auto routes::index(http::Request&) -> http::Response
{
	return R {}.content(
	    "<!DOCTYPE html>"
	    "<html>"
	    "<body>"
	    "INDEX PAGE"
	    "<img src=\"/The_army_of_titanium_dioxide_nanotubes(1).jpg\"/>"
	    "</body>"
	    "</html>"
	);
}
