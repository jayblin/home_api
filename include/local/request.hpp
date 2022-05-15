#include <string_view>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef REQUEST_H_
#define REQUEST_H_

struct Request
{
	Request(const std::string_view raw_request);
	
	~Request();

	std::string method;
	std::string host;
	std::string path;
	std::string query;
};

namespace request
{
	/* auto explode_query(Request& request) -> std::vector<std::string> */
	/* { */
	/* 	/1* request.query *1/ */

	/* 	// split params by &; */
		

	/* 	return {}; */
	/* } */
}

#endif // REQUEST_H_

