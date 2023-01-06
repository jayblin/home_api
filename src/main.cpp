#include "config.h"
#include "http/headers_parser.hpp"
#include "http/request.hpp"
#include "http/request_parser.hpp"
#include "route_map.hpp"
#include "routes/index.hpp"
#include "sock/buffer.hpp"
#include "sock/socket_factory.hpp"
#include "sock/utils.hpp"
#include "utils.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

/**
 * If status of `response` is NOT_FOUND than this function will check if
 * a file was requested and will return a filestream.
 *
 * @return std::ifstream Filestream of requested resource.
 */
std::ifstream
    try_get_file(const http::Request& request, http::Response& response)
{
	if (http::Code::NOT_FOUND == response.code() &&
	    std::string::npos == request.target.find(".."))
	{
		const auto p = std::filesystem::path(PUBLIC_DIR + request.target);

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

			return std::ifstream {p, std::ifstream::binary};
		}
	}

	return std::ifstream {};
}

int main(int argc, char const* argv[])
{
	auto& factory = sock::SocketFactory::instance();

	{
		auto server_sock = factory
			.wrap({
				.domain = sock::Domain::INET,
				.type = sock::Type::STREAM,
				.protocol = sock::Protocol::TCP,
				.port = "13908",
				.flags = sock::Flags::PASSIVE,
			})
			.with([](auto& sock) {
				if (sock.status() != sock::Status::GOOD)
				{
					sock::log_error(
						std::string{"Server error "}
						+ std::string{sock::str_status(sock.status())}
					);
				}
			})
			.create()
		;
		server_sock
			.option(sock::Option::REUSEADDR, 1)
			.bind()
			.listen(10)
		;

		auto& map = RouteMap::instance();

		while (1)
		{
			auto connection = server_sock.accept();
			http::Request request;
			http::RequestParser parser{request};
			sock::Buffer buffer;

			do
			{
				connection.receive(buffer);

				http::Parsor parsor{buffer.buffer()};
				parser.parse(parsor);
			}
			while (buffer.received_size() > 0 && !parser.is_finished());

			auto response = map.match_method_with_request(request);

			auto requested_file = try_get_file(request, response);

			auto ss = std::stringstream {};

			ss << "HTTP/1.1 " << http::code_to_int(response.code()) << " "
			   << http::code_to_str(response.code()) << "\n"
			   << "Content-type: "
			   << http::content_type_to_str(response.content_type())
			   << "; charset=" << http::charset_to_str(response.charset()) << "\n"
			   << "\n";

			connection.send(ss.view());

			if (response.content().length() > 0)
			{
				connection.send(response.content());
			}
			else if (requested_file)
			{
				constexpr auto BODY_LEN = 1024 * 1024;
				char buf[BODY_LEN];

				do
				{
					std::memset(buf, 0, BODY_LEN);

					requested_file.read(buf, BODY_LEN - 1);

					const auto count = requested_file.gcount();

					if (count > 0)
					{
						connection.send(std::string_view{
							buf,
							static_cast<std::string_view::size_type>(count)
						});
					}
				} while (requested_file.good() && !requested_file.eof());
			}

			connection.send("");

			connection.shutdown();
		}
	}

	return 0;
}
