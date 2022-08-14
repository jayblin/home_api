#include "config.h"
#include "socket.hpp"
#include "route_map.hpp"

#include "http/headers_parser.hpp"
#include "http/request_line_parser.hpp"
#include "http/request.hpp"

#include "routes/index.hpp"
#include "utils.hpp"

#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

/**
 * If status of `response` is NOT_FOUND than this function will check if
 * a file was requested and will return a filestream.
 *
 * @return std::ifstream Filestream of requested resource.
 */
std::ifstream try_get_file(
	const http::Request& request,
	http::Response& response
)
{
	if (
		http::Code::NOT_FOUND == response.code()
		&& std::string::npos == request.path.find("..")
	)
	{
		const auto p = std::filesystem::path(PUBLIC_DIR + request.path);
		
		if (std::filesystem::exists(p))
		{
			response.code(http::Code::OK);

			if (0 == p.extension().compare(".ico"))
			{
				response.content_type(http::ContentType::IMG_X_ICON);
			}
			else if (0 == p.extension().compare(".jpg"))
			{
				response.content_type(http::ContentType::IMG_JPG);
			}

			return std::ifstream{p, std::ifstream::binary};
		}
	}

	return std::ifstream{};
}

/**
 * Tries to send a response through socket.
 * Logs error on failure.
 */
void try_send(SOCKET& socket, const char* buff, const size_t count)
{
	auto send_result = send(
		socket,
		buff,
		count,
		0
	);

	if (send_result == SOCKET_ERROR)
	{
		log_error("send failed");
	}
}

int main(int argc, char const* argv[])
{
	if (!init_socket_lib())
	{
		return 1;
	}

	auto sock = create_server_socket();

	auto map = RouteMap{
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

		auto response = map.match_method_with_request(request);

		auto requested_file = try_get_file(request, response);

		auto ss = std::stringstream{};

		ss
			<< "HTTP/1.1 " << http::code_to_int(response.code()) << " " << http::code_to_str(response.code()) << "\n"
			<< "Content-type: " << http::content_type_to_str(response.content_type()) << "; charset=" << http::charset_to_str(response.charset()) << "\n"
			<< "\n"
		;

		try_send(
			client_sock,
			ss.view().data(),
			ss.view().size()
		);

		constexpr auto BODY_LEN = 1024 * 1024;
		char buf[BODY_LEN];

		if (response.content().length() > 0)
		{
			try_send(
				client_sock,
				response.content().data(),
				response.content().length()
			);
		}
		else if (requested_file)
		{
			do
			{
				std::memset(buf, 0, BODY_LEN);

				requested_file.read(buf, BODY_LEN - 1);

				const auto count = requested_file.gcount();

				try_send(client_sock, buf, count);
			}
			while (requested_file.good() && !requested_file.eof());
		}

		try_send(client_sock, "", 0);

		const auto shutdown_result = shutdown(client_sock, SD_SEND);
		if (shutdown_result == SOCKET_ERROR) {
			log_error("shutdown failed");
		}

		closesocket(client_sock);
	}

	closesocket(sock);

	WSACleanup();
}
