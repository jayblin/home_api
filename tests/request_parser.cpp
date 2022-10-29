#include <gtest/gtest.h>
#include <string_view>
#include "http/headers.hpp"
#include "http/headers_parser.hpp"
#include "http/parsor.hpp"
#include "http/request.hpp"
#include "http/request_parser.hpp"
#include "http/status_line_parser.hpp"

TEST(RequestParser, Literal)
{
	http::Parsor parsor{
		"GET /hello.txt?display=all HTTP/1.1\r\n"
		"Host: www.example.com\r\n"
		"\r\n"
	};
	http::Request request;
	http::RequestParser rp;

	rp.parse(request, parsor);

	EXPECT_STREQ("GET", request.method.data());
	EXPECT_STREQ("/hello.txt", request.target.data());
	EXPECT_EQ(1, request.http_version);
	EXPECT_STREQ("display=all", request.query.data());
	EXPECT_STREQ("www.example.com", request.headers.host.data());
	EXPECT_EQ(0, request.headers.content_length);
	EXPECT_STREQ("", request.body.data());
}

TEST(RequestParser, LiteralPost)
{
	http::Parsor parsor{
		"POST /hello.txt HTTP/1.1\r\n"
		"Host: www.example.com\r\n"
		"Content-Length: 9\r\n"
		"\r\n"
		"Some body"
	};
	http::Request request;
	http::RequestParser rp;

	rp.parse(request, parsor);

	EXPECT_STREQ("POST", request.method.data());
	EXPECT_STREQ("/hello.txt", request.target.data());
	EXPECT_EQ(1, request.http_version);
	EXPECT_EQ(9, request.headers.content_length);
	EXPECT_STREQ("Some body", request.body.data());
}

TEST(RequestParser, LiteralPostLineBreaks)
{
	http::Parsor parsor{
		"POST /hello.txt HTTP/1.1\r\n"
		"Host: www.example.com\r\n"
		"Content-Length: 77\r\n"
		"\r\n"
		"Now this is the story\r\n"
		"All about how\n"
		"My life got flipped, turned upside down\r"
	};
	http::Request request;
	http::RequestParser rp;

	rp.parse(request, parsor);

	EXPECT_STREQ("POST", request.method.data());
	EXPECT_STREQ("/hello.txt", request.target.data());
	EXPECT_EQ(1, request.http_version);
	EXPECT_EQ(77, request.headers.content_length);
	EXPECT_STREQ("Now this is the story\r\nAll about how\nMy life got flipped, turned upside down\r", request.body.data());
}

TEST(RequestParser, MultipleReceives)
{
	http::Parsor parsor1{
		"POST /hello.txt HTTP/1.1\r\n"
		"Host: www.example.com\r\n"
		"Content-Length: 77\r\n"
		"\r\n"
	};
	http::Request request;
	http::RequestParser rp;

	rp.parse(request, parsor1);

	EXPECT_STREQ("POST", request.method.data());
	EXPECT_STREQ("/hello.txt", request.target.data());
	EXPECT_EQ(1, request.http_version);
	EXPECT_EQ(77, request.headers.content_length);
	EXPECT_STREQ("", request.body.data());

	http::Parsor parsor2{
		"Now this is the story\r\n"
	};

	rp.parse(request, parsor2);

	EXPECT_STREQ("", request.body.data());

	http::Parsor parsor3{
		"All about how\n"
		"My life go"
	};

	rp.parse(request, parsor3);

	EXPECT_STREQ("", request.body.data());

	http::Parsor parsor4{
		"t flipped, turned upside down\r"
	};

	rp.parse(request, parsor4);

	EXPECT_STREQ("Now this is the story\r\nAll about how\nMy life got flipped, turned upside down\r", request.body.data());
}
