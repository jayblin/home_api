#include "socket.hpp"
#include "request.hpp"
#include "response.hpp"
#include "route-map.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <functional>
#include <concepts>

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

auto list_ingredients(Request& request) -> Response
{
	const auto req = GetRequest{request};

	return {
		.code = http::Code::OK,
		.content = "[{\"title\": \"Соль\"}]",
		.content_type = "application/json; charset=UTF-8",
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
		{ http::Method::GET, "/api/ingredients", &list_ingredients }
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
		char buffer[l] = "";

		auto recv_result = recv(client_sock, buffer, l, 0);

		auto request = Request{buffer};

		const auto response = map.match_method_with_request(request);

		auto ss = std::stringstream{};

		ss
			<< "HTTP/1.1 " << static_cast<int>(response.code) << " "
			<< http::code_to_str(response.code) << "\n"
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

		const auto shutdown_result = shutdown(client_sock, SD_SEND);
		if (shutdown_result == SOCKET_ERROR) {
			log_error("shutdown failed");
			closesocket(client_sock);
		}

		closesocket(client_sock);
	}

	closesocket(sock);

	WSACleanup();
}

