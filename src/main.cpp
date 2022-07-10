#include "http/headers_parser.hpp"
#include "http/request_line_parser.hpp"
#include "socket.hpp"
#include "request.hpp"
#include "route-map.hpp"
#include "routes/index.hpp"
#include "routes/ingredients.hpp"

#include <sstream>
#include <string>

int main(int argument, char const* argv[])
{
	if (!init_socket_lib())
	{
		return 1;
	}

	auto sock = create_server_socket();

	auto map = RouteMap{
		{
			http::Method::GET,
			"/api/ingredients",
			&routes::ingredients::list
		},
		{
			http::Method::POST,
			"/api/ingredients",
			&routes::ingredients::post
		},
		{ http::Method::GET, "/", &routes::index }
	};

	constexpr auto MAX = 1024 * 10;
	static_assert(MAX >= 1024);

	char recv_buff[MAX];

	while (1)
	{
		auto client_sock = accept(sock, NULL, NULL);

		if (client_sock == INVALID_SOCKET)
		{
			log_error("accept failed");
			closesocket(client_sock);
			continue;
		}

		memset(recv_buff, 0, MAX);

		size_t n = 0;

		auto finished = false;

		http::RequestLineParser rl_parser;
		http::HeadersParser h_parser;
		std::string body = "";

		// MAX - 1, because 0 terminates string, and so i dont have to manualy
		// set last char to 0.
		while ((n = recv(client_sock, recv_buff, MAX-1, 0)) > 0)
		{
			if (!h_parser.is_finished())
			{
				for (size_t i = 0; i < n; i++)
				{
					if (!rl_parser.is_finished())
					{
						rl_parser.parse(recv_buff, i);
					}
					else if (!h_parser.is_finished())
					{
						h_parser.parse(recv_buff, i);
					}
				}
			}
			else if (h_parser.content_length > 0)
			{
				body += std::string(recv_buff, n);
			}

			memset(recv_buff, 0, MAX);

			if (
				(h_parser.is_finished() && h_parser.content_length == 0)
				|| (body.length() >= h_parser.content_length)
			)
			{
				break;
			}
		}

		http::Request request;

		request.method 					= std::move(rl_parser.method);
		request.path 					= std::move(rl_parser.path);
		request.query 					= std::move(rl_parser.query);
		request.headers.content_length 	= h_parser.content_length;
		request.headers.host 			= std::move(h_parser.host);
		request.body 					= std::move(body);

		const auto response = map.match_method_with_request(request);

		auto ss = std::stringstream{};

		ss
			<< "HTTP/1.1 " << http::code_to_int(response.code) << " " << http::code_to_str(response.code) << "\n"
			<< "Content-type: " << http::content_type_to_str(response.content_type) << "; charset=" << http::charset_to_str(response.charset) << "\n"
			<< "\n"
			<< response.content
		;

		const auto b = ss.str();

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
		}

		closesocket(client_sock);
	}

	closesocket(sock);

	WSACleanup();
}
