#include "server/server.hpp"
#include "http/forward.hpp"
#include "http/headers_parser.hpp"
#include "http/parsor.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/status_line_parser.hpp"
#include "local/cmake_vars.h"
#include "local/route_map.hpp"
#include "sock/socket_factory.hpp"
#include "sock/socket_wrapper.hpp"
#include "sock/utils.hpp"
#include "sqlw/connection.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <utility>

/**
 * If status of `response` is NOT_FOUND than this function will check if
 * a file was requested and will return a filestream.
 *
 * @return std::ifstream Filestream of requested resource.
 */
static std::ifstream
    try_get_file(const http::Request& request, http::Response& response)
{
	if (http::Code::NOT_FOUND == response.code()
	    && std::string::npos == request.target.find(".."))
	{
		const auto p = std::filesystem::path {PUBLIC_DIR} / request.target;

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

static std::string headers(const http::Response& response)
{
	auto ss = std::stringstream {};

	ss << "HTTP/1.1 " << http::code_to_int(response.code()) << " "
	   << http::code_to_str(response.code()) << "\r\n"
	   << "Content-type: "
	   << http::content_type_to_str(response.content_type())
	   << "; charset=" << http::charset_to_str(response.charset()) << "\r\n"
	   << "\r\n";

	return ss.str();
}

static auto perform_response(
    const RouteMap& map,
    http::Request& request,
    sock::SocketWrapper& client,
	RouteMap::BodyGetter& get_body
)
{
	auto response = map.match_method_with_request(request, get_body);

	auto requested_file = try_get_file(request, response);

	client.send(headers(response));

	if (response.content().length() > 0)
	{
		client.send(response.content());
	}
	else if (requested_file)
	{
		/**
		 * @todo: inspect.
		 */
		constexpr auto BODY_LEN = 1024 * 1024;
		char buf[BODY_LEN];

		do
		{
			std::memset(buf, 0, BODY_LEN);

			requested_file.read(buf, BODY_LEN - 1);

			const auto count = requested_file.gcount();

			if (count > 0)
			{
				client.send(std::string_view {
				    buf,
				    static_cast<std::string_view::size_type>(count)});
			}
		}
		while (requested_file.good() && !requested_file.eof());
	}
}

void server::start(sock::Address address, sqlw::Connection* db_connection)
{
	using namespace std::chrono_literals;

	Server::instance().db_connection = db_connection;

	auto server_sock = sock::SocketFactory::instance()
	                       .wrap({
	                           .domain = sock::Domain::UNSPEC,
	                           .type = sock::Type::STREAM,
	                           .protocol = sock::Protocol::TCP,
	                           .flags = sock::Flags::PASSIVE,
	                       })
	                       .with(
	                           [](sock::Socket& s)
	                           {
		                           if (s.status() != sock::Status::GOOD)
		                           {
			                           std::cout
			                               << "Server error "
			                               << sock::str_status(s.status())
			                               << " - " << sock::error() << '\n';
		                           }
	                           }
	                       )
	                       .create();

	server_sock.option(sock::Option::REUSEADDR, 1)
	    .option(sock::Option::RCVTIMEO, 1000ms)
	    .bind(address)
	    .listen(10);

	const auto& map = RouteMap::instance();

	while (1)
	{
		auto conn = server_sock.accept();

		conn.callback(
		    [](sock::Socket& s)
		    {
			    if (s.status() != sock::Status::GOOD)
			    {
				    std::cout << "Connection error "
				              << sock::str_status(s.status()) << " - "
				              << sock::error() << '\n';
			    }
		    }
		);

		std::thread client_thread {
		    [&map](sock::SocketWrapper&& client_connection)
		    {
			    http::Request request;
				std::stringstream body_stream;

				RouteMap::BodyGetter get_body =
			        [&client_connection, &body_stream, &request]() mutable
			    {
				    sock::Buffer buffer;

				    do
				    {
						client_connection.receive(buffer);

					    if (buffer.received_size() <= 0
					        || client_connection.status() != sock::Status::GOOD
					        || request.headers.content_length <= 0
					        || body_stream.view().length()
					               >= request.headers.content_length)
					    {
						    break;
					    }

					    body_stream << buffer.view();
				    }
				    while (1);

				    return body_stream.str();
			    };

			    http::StatusLineParser slp;
			    http::HeadersParser hp;
			    sock::Buffer buffer;

			    do
			    {
				    client_connection.receive(buffer);

				    if (buffer.received_size() <= 0
				        || client_connection.status() != sock::Status::GOOD)
				    {
					    break;
				    }

				    http::Parsor parsor {buffer.view()};

				    if (!slp.is_finished())
				    {
					    slp.parse(request, parsor);
				    }

				    if (slp.is_finished() && !hp.is_finished())
				    {
					    hp.parse(request.headers, parsor);
				    }

				    if (slp.is_finished() && hp.is_finished())
				    {
						if (!parsor.is_end())
						{
							body_stream << parsor.view().substr(parsor.cur_pos());
						}

					    perform_response(map, request, client_connection, get_body);

					    request = http::Request {};
					    slp.reset();
					    hp.reset();
				    }
			    }
			    while (1);

			    client_connection.shutdown();
		    },
		    std::move(conn)};

		client_thread.detach();
	}
}
