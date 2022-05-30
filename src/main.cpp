#include "socket.hpp"
#include "request.hpp"
#include "response.hpp"
#include "utils.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <functional>
#include <concepts>
#include <type_traits>

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
	auto is_inited = jsocket::init_socket_lib();

	if (!is_inited)
	{
		return 1;
	}

	auto constr_args = jsocket::ConstructorArgs{
		.domain = jsocket::AddressFamily::INET,
		.type = jsocket::SocketType::STREAM,
		.protocol = jsocket::Protocol::TCP,
	};

	/* auto address = sockaddr_in{ */
	/* 	.sin_family = AF_INET, */
	/* 	.sin_port = htons(13098), */
	/* 	.sin_addr = { */
	/* 		.s_addr = INADDR_ANY, */
	/* 	}, */
	/* }; */

	auto address = jsocket::Address{
		.family = jsocket::AddressFamily::INET,
		.port = "13908",
		.addr = jsocket::BindingAddress::ANY,
	};

	/* auto sock = Socket{constr_args}; */
	auto sock = jsocket::Socket{constr_args};

	sock
		.bind(address)
		.set_option({
			.level = jsocket::OptionLevel::SOCKET,
			.name = jsocket::OptionName::REUSEADDR,
			.value = 1
		})
		.set_option({
			.level = jsocket::OptionLevel::SOCKET,
			.name = jsocket::OptionName::KEEPALIVE,
			.value = 1
		})
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

			return 1;
		}

		auto new_socket = sock.accept(address);

		const int l = 1024 * 5;
		char raw_request[l] = {};

		new_socket.read(&raw_request, l);
		
		auto request = Request{raw_request};

		const auto response = match_method_with_request(request, map);

		auto ss = std::stringstream{};

		ss 	<< "HTTP/1.1 " << static_cast<int>(response.code) << " " << response::status(response) << "\n"
			<< "Content-type: " << response.content_type << "\n"
			<< "\n"
			<< response.content
		;

		new_socket.send(ss.view());
	}

	jsocket::close_socket_lib();
}

