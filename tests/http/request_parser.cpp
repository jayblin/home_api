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
	http::RequestParser rp{request};

	rp.parse(parsor);

	EXPECT_EQ("GET", request.method);
	EXPECT_EQ("/hello.txt", request.target);
	EXPECT_EQ(1, request.http_version);
	EXPECT_EQ("display=all", request.query);
	EXPECT_EQ("www.example.com", request.headers.host);
	EXPECT_EQ(0, request.headers.content_length);
	EXPECT_EQ("", request.body);
	EXPECT_TRUE(rp.is_finished());
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
	http::RequestParser rp{request};

	rp.parse(parsor);

	EXPECT_EQ("POST", request.method);
	EXPECT_EQ("/hello.txt", request.target);
	EXPECT_EQ(1, request.http_version);
	EXPECT_EQ(9, request.headers.content_length);
	EXPECT_EQ("Some body", request.body);
	EXPECT_TRUE(rp.is_finished());
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
	http::RequestParser rp{request};

	rp.parse(parsor);

	EXPECT_EQ("POST", request.method);
	EXPECT_EQ("/hello.txt", request.target);
	EXPECT_EQ(1, request.http_version);
	EXPECT_EQ(77, request.headers.content_length);
	EXPECT_EQ("Now this is the story\r\nAll about how\nMy life got flipped, turned upside down\r", request.body);
	EXPECT_TRUE(rp.is_finished());
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
	http::RequestParser rp{request};

	rp.parse(parsor1);

	EXPECT_EQ("POST", request.method);
	EXPECT_EQ("/hello.txt", request.target);
	EXPECT_EQ(1, request.http_version);
	EXPECT_EQ(77, request.headers.content_length);
	EXPECT_EQ("", request.body);
	EXPECT_FALSE(rp.is_finished());

	http::Parsor parsor2{
		"Now this is the story\r\n"
	};

	rp.parse(parsor2);

	EXPECT_EQ("", request.body);
	EXPECT_FALSE(rp.is_finished());

	http::Parsor parsor3{
		"All about how\n"
		"My life go"
	};

	rp.parse(parsor3);

	EXPECT_EQ("", request.body);
	EXPECT_FALSE(rp.is_finished());

	http::Parsor parsor4{
		"t flipped, turned upside down\r"
	};

	rp.parse(parsor4);

	EXPECT_EQ("Now this is the story\r\nAll about how\nMy life got flipped, turned upside down\r", request.body);
	EXPECT_TRUE(rp.is_finished());
}
