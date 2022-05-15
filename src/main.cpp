#include "socket.hpp"
#include "request.hpp"
#include "response.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <functional>

using RouteMap = std::unordered_map<
	std::string,
	Response (*)(Request&)
>;

namespace api::ingredients
{
	struct Ingredient
	{
		int id;
		std::string title;
	};

	struct GetRequest
	{
		int page = 1;
		int id = -1;

		GetRequest(Request& request)
		{
			/* request.query */
		}
	};

	auto list() -> std::vector<Ingredient>
	{
		return {};
	}

	auto get(Request& request) -> Response
	{
		const auto req = GetRequest{request};

		return {
			.code=HttpCode::OK,
			.content="[{\"title\": \"Соль\"}]",
			.content_type="application/json; charset=UTF-8",
		};
	}
}

auto match_method_with_request(
	Request& request,
	RouteMap& map
) -> Response
{
	auto& path = request.path;
	auto _i = 0;
	auto i = 0;

	while (i != std::string::npos && i < path.size() && _i < 100)
	{
		_i++;
		const auto last_i = i;
		i = path.find("/", i+1);

		const auto route = path.substr(0, i);

		const auto func = map[request.method + ":" + route];

		if (func)
		{
			return func(request);
		}
	}

	return {
		.code=HttpCode::NotFound,
		.content="",
		.content_type="text/html; charset=UTF-8",
	};
}

int main(int argument, char const* argv[])
{
	auto constr_args = ConstructorArgs{
		.domain = AF_INET,
		.type = SOCK_STREAM,
		.protocol = 0,
	};

	auto option = Option{
		.level = SOL_SOCKET,
		.name = SO_REUSEADDR|SO_REUSEPORT,
		.value = 1
	};

	auto address = sockaddr_in{
		.sin_family = AF_INET,
		.sin_port = htons(13098),
		.sin_addr = {
			.s_addr = INADDR_ANY,
		},
	};

	auto sock = Socket{constr_args};

	sock.set_option(option)
		.bind(address)
		.listen(10)
	;

	auto map = RouteMap{
		{ "GET:/api/ingredients", &api::ingredients::get },
	};

	while (1)
	{
		auto& errors = sock.errors();

		if (errors.size() > 0)
		{
			for (auto& error : errors)
			{
				std::cout << error << "\n";
			}
		}

		auto new_socket = sock.accept(address);

		char raw_request[1024] = {0};

		new_socket.read(&raw_request, 1024);
		
		auto request = Request{raw_request};

		CLOG(request.path << " : " << request.query);

		const auto response = match_method_with_request(request, map);

		auto ss = std::stringstream{};

		ss 	<< "HTTP/1.1 " << static_cast<int>(response.code) << " " << response::status(response) << "\n"
			<< "Content-type: " << response.content_type << "\n"
			<< "\n"
			<< response.content
		;

		new_socket.send(ss.view());
	}
}

