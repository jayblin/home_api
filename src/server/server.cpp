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
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
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
		const auto p =
		    std::filesystem::path {PUBLIC_DIR} / request.target.substr(1);

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

static std::string compile(const http::Response& response)
{
	auto ss = std::stringstream {};

	const std::string_view headers = response.headers();

	ss << "HTTP/1.1 " << http::code_to_int(response.code()) << " "
	   << http::code_to_str(response.code()) << "\r\n"
	   << "Content-type: "
	   << http::content_type_to_str(response.content_type())
	   << "; charset=" << http::charset_to_str(response.charset()) << "\r\n"
	   << headers << (headers.length() > 0 ? "\r\n" : "")
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

	client.send(compile(response));

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
			/* std::memset(buf, 0, BODY_LEN); */

			requested_file.read(buf, BODY_LEN);

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
		const auto t = std::chrono::system_clock::to_time_t(
		    std::chrono::system_clock::now()
		);

		/* std::cout << '\n' */
		/*           << std::put_time(std::localtime(&t), "%X %d.%m.%Y") << '\n'; */

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

			    // @todo: find out what is mutable.
			    RouteMap::BodyGetter get_body =
			        [&client_connection, &body_stream, &request]() mutable
			    {
				    if (request.headers.content_length <= 0)
				    {
					    // @todo: set some kinda status?
					    return body_stream.str();
				    }

				    sock::Buffer buffer;

					int i = 0;
				    do
					{
						i++;
					    // @todo: inform client if response body didn't fit in
					    // buffer.
					    client_connection.receive(buffer);
					    body_stream << buffer.view();
				    }
				    while (buffer.received_size() > 0
				           && client_connection.status() == sock::Status::GOOD
				           && body_stream.view().length()
				                  < request.headers.content_length);

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

				    request.raw += buffer.view();

				    http::Parsor parsor {request.raw};

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
						    body_stream
						        << parsor.view().substr(parsor.cur_pos());
					    }

						/* std::cout << request.raw << '\n'; */
					    /* std::cout << "HTTP v" << request.http_version << " [" */
					    /*           << request.method << "] " << request.target */
					    /*           << " " << request.query << '\n'; */

					    perform_response(
					        map,
					        request,
					        client_connection,
					        get_body
					    );

					    request = http::Request {};
					    slp.reset();
					    hp.reset();
					    body_stream.str("");
					    body_stream.clear();

						/* break; */
				    }
			    }
			    while (1);

			    client_connection.shutdown();
		    },
		    std::move(conn)};

		client_thread.detach();
	}
}
