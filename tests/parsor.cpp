#include <gtest/gtest.h>
#include "http/parsor.hpp"

TEST(Parsor, MainTest)
{
	{
		http::Parsor parsor{""};

		EXPECT_EQ(-1, parsor.cur_pos());

		parsor.advance();
		EXPECT_EQ(-1, parsor.cur_pos());

		parsor.advance(14);
		EXPECT_EQ(-1, parsor.cur_pos());
	}

	{
		http::Parsor parsor{"Hello world"};

		EXPECT_EQ(0, parsor.cur_pos());
		EXPECT_EQ('H', parsor.cur_char());

		parsor.advance();
		EXPECT_EQ(1, parsor.cur_pos());
		EXPECT_EQ('e', parsor.cur_char());

		parsor.advance(14);
		EXPECT_EQ(10, parsor.cur_pos());
		EXPECT_EQ('d', parsor.cur_char());
	}

	{
		http::Parsor parsor{"Abnormal <br>\n\n\n"};

		EXPECT_EQ(0, parsor.cur_pos());
		EXPECT_EQ('A', parsor.cur_char());

		parsor.advance();
		EXPECT_EQ(1, parsor.cur_pos());
		EXPECT_EQ('b', parsor.cur_char());

		parsor.advance(145);
		EXPECT_EQ(15, parsor.cur_pos());
		EXPECT_EQ('\n', parsor.cur_char());
	}

	{
		http::Parsor parsor{
			"multiline\r\n"
			"text\r\n"
			"\r\n"
		};

		EXPECT_EQ(0, parsor.cur_pos());
		EXPECT_EQ('m', parsor.cur_char());

		parsor.advance(9);
		EXPECT_EQ(9, parsor.cur_pos());
		EXPECT_EQ('\r', parsor.cur_char());

		parsor.advance(100);
		EXPECT_EQ(18, parsor.cur_pos());
		EXPECT_EQ('\n', parsor.cur_char());
	}
}

