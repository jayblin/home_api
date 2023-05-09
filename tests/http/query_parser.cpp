#include "http/query_parser.hpp"
#include "http/parsor.hpp"
#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

GTEST_TEST(QueryParser, can_parse_no_value)
{
	std::string_view empty;
	http::Parsor parsor {empty};
	http::Request request;
	http::QueryParser qp;

	std::unordered_map<std::string_view, http::QueryValue> values =
	    qp.parse(parsor);

	EXPECT_TRUE(qp.is_finished());
}


GTEST_TEST(QueryParser, can_parse_empty_value)
{
	http::Parsor parsor {""};
	http::Request request;
	http::QueryParser qp;

	std::unordered_map<std::string_view, http::QueryValue> values =
	    qp.parse(parsor);

	EXPECT_TRUE(qp.is_finished());
}

GTEST_TEST(QueryParser, can_parse_singular_value)
{
	http::Parsor parsor {"display=all"};
	http::Request request;
	http::QueryParser qp;

	std::unordered_map<std::string_view, http::QueryValue> values =
	    qp.parse(parsor);

	EXPECT_TRUE(qp.is_finished());

	EXPECT_STREQ("all", std::string{values["display"].value}.c_str());
}

GTEST_TEST(QueryParser, can_parse_multiple_values)
{
	http::Parsor parsor {"display=all&count=123&json=true"};
	http::Request request;
	http::QueryParser qp;

	std::unordered_map<std::string_view, http::QueryValue> values =
	    qp.parse(parsor);

	EXPECT_TRUE(qp.is_finished());

	EXPECT_STREQ("all", std::string{values["display"].value}.c_str());
	EXPECT_STREQ("123", std::string{values["count"].value}.c_str());
	EXPECT_STREQ("true", std::string{values["json"].value}.c_str());
}

GTEST_TEST(QueryParser, can_parse_list_value)
{
	http::Parsor parsor {"dbm=sqlite&item=1&item=3&item=4"};
	http::Request request;
	http::QueryParser qp;

	std::unordered_map<std::string_view, http::QueryValue> values =
	    qp.parse(parsor);

	EXPECT_TRUE(qp.is_finished());

	EXPECT_STREQ("sqlite", std::string{values["dbm"].value}.c_str());

	EXPECT_EQ(3, values["item"].values.size());
	EXPECT_STREQ("1", std::string{values["item"].values.at(0)}.c_str());
	EXPECT_STREQ("3", std::string{values["item"].values.at(1)}.c_str());
	EXPECT_STREQ("4", std::string{values["item"].values.at(2)}.c_str());
}

GTEST_TEST(QueryParser, can_handle_weird_value)
{
	http::Parsor parsor {"1ffe(*33&03=44&&&-==="};
	http::Request request;
	http::QueryParser qp;

	std::unordered_map<std::string_view, http::QueryValue> values =
	    qp.parse(parsor);

	EXPECT_TRUE(qp.is_finished());
}
