#include "socket.hpp"
#include "request.hpp"
#include "response.hpp"
#include "utils.hpp"

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
	if (!init_socket_lib())
	{
		return 1;
	}

	auto sock = create_server_socket();

	auto map = RouteMap{
		{ "GET:/api/ingredients", &api::ingredients::get },
	};


	while (1)
	{
		auto client_sock = accept(sock, NULL, NULL);

		if (client_sock == INVALID_SOCKET)
		{
			log_error("accept failed");
			closesocket(client_sock);
			continue;
		}

		const size_t l = 1024 * 5;
		char buffer[l];

		auto recv_result = recv(client_sock, buffer, l, 0);

		auto request = Request{buffer};

		const auto response = match_method_with_request(request, map);

		auto ss = std::stringstream{};

		ss 	<< "HTTP/1.1 " << static_cast<int>(response.code) << " " << response::status(response) << "\n"
			<< "Content-type: " << response.content_type << "\n"
			<< "\n"
			<< response.content
		;

		auto send_result = send(
			client_sock,
			ss.view().data(),
			ss.view().size(),
			0
		);

		if (send_result == SOCKET_ERROR)
		{
			log_error("send failed");
		}

		closesocket(client_sock);
	}

	closesocket(sock);

	WSACleanup();
}

