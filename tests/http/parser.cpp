#include "http/parser.hpp"
#include "http/headers_parser.hpp"
#include "http/parsor.hpp"
#include <gtest/gtest.h>

TEST(Parser, Literal)
{
	http::Parsor parsor {
	    "GET /api/backend/cart?domain=shop.nag.ru&utm_source=shopnext&requestID=3nakubgwqnp HTTP/2\r\n"
	    "Host: b2b74.nag.ru\r\n"
	    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:107.0) Gecko/20100101 Firefox/107.0\r\n"
	    "Accept: application/json\r\n"
	    "Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3\r\n"
	    "Accept-Encoding: gzip, deflate, br\r\n"
	    "Referer: https://shop.nag.ru/\r\n"
	    "Content-Type: application/json;charset=utf-8\r\n"
	    "Origin: https://shop.nag.ru\r\n"
	    "Connection: keep-alive\r\n"
	    "Cookie: _ga_3C1X7M2J99=GS1.1.1674744598.4.1.1674745048.0.0.0; rrpvid=636486732148612; _ga=GA1.2.1690264112.1662905906; rcuid=631ade91e73c65e643f20ea9; _ym_uid=16814045542578031; _ym_d=1681404554; roistat_visit=5179388; order_cookie=d3798b19-cd11-44c7-b2e2-ffd79f6da0f6; ips4_IPSSessionFront=ntu7quf95ooc2al4evjl11g084; ips4_ipsTimezone=Asia/Yekaterinburg; grigoriyshop_b2b=kc9cf3ueiio0t41cor2jnichm1\r\n"
	    "Sec-Fetch-Dest: empty\r\n"
	    "Sec-Fetch-Mode: cors\r\n"
	    "Sec-Fetch-Site: same-site\r\n"
	    "TE: trailers\r\n"};
	http::Parser parser;

	http::parser::Context<3, 2> ctx {
	    {"grigoriyshop_b2b", "_ga_3C1X7M2J99", "rrpvid"},
	    {";", "\r\n"},
	    "=",
	    "Cookie:",
    };
	const auto cookies = parser.parse(parsor, ctx);

	EXPECT_EQ(3, cookies.size());

	EXPECT_EQ(std::string {"kc9cf3ueiio0t41cor2jnichm1"}, cookies[0]);
	EXPECT_EQ(
	    std::string {"GS1.1.1674744598.4.1.1674745048.0.0.0"},
	    cookies[1]
	);
	EXPECT_EQ(std::string {"636486732148612"}, cookies[2]);
}
