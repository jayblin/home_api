#include <gtest/gtest.h>
#include "http/parsor.hpp"
#include "http/request.hpp"
#include "http/status_line_parser.hpp"

TEST(RequestLineParser, Literal)
{
	http::Parsor parsor{"POST /api/throng?var1=val1&var2=val2 HTTP/1.1\r\n"};
	http::Request request;
	http::StatusLineParser rlp;

	rlp.parse(request, parsor);

	EXPECT_EQ("POST", request.method);
	EXPECT_EQ("/api/throng", request.target);
	EXPECT_EQ("var1=val1&var2=val2", request.query);
	EXPECT_EQ(1, request.http_version);
}

TEST(RequestLineParser, CharPointer)
{
	char str[1024] = "GET / HTTP/2\r\n";

	http::Parsor parsor{str};
	http::Request request;
	http::StatusLineParser rlp;

	rlp.parse(request, parsor);

	EXPECT_EQ("GET", request.method);
	EXPECT_EQ("/", request.target);
	EXPECT_EQ("", request.query);
	EXPECT_EQ(2, request.http_version);
}
