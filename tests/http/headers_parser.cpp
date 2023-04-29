#include <gtest/gtest.h>
#include "http/headers.hpp"
#include "http/headers_parser.hpp"
#include "http/parsor.hpp"
#include "http/request.hpp"

TEST(HeadersParser, Literal)
{
	http::Parsor parsor{
		"Content-Length: 20\r\n"
		"Host: www.example.com\r\n"
		"\r\n"
	};
	http::Headers headers;
	http::HeadersParser hp;

	hp.parse(headers, parsor);

	EXPECT_EQ("www.example.com", headers.host);
	EXPECT_EQ(20, headers.content_length);
}

TEST(HeadersParser, CharPointer)
{
	char str[1024] = "Content-Length: 0\r\n"
		"Host: www.hello.net\r\n"
		"\r\n"
	;

	http::Parsor parsor{str};
	http::Headers headers;
	http::HeadersParser hp;

	hp.parse(headers, parsor);

	EXPECT_EQ("www.hello.net", headers.host);
	EXPECT_EQ(0, headers.content_length);
}

