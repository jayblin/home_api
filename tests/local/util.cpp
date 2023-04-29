#include "http/response.hpp"
#include "local/utils.hpp"
#include "nlohmann/json_fwd.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include <gtest/gtest.h>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>

GTEST_TEST(create_errors_json, does_create_correct_json)
{
	std::string sql = utils::create_errors_json({
	    R"(\"Feed the Eidolon Ally, and it will be transformed.\" —Eris Morn)",
	    "'To rend one's enemies is to see them not as equals, but objects—hollow of spirit and meaning.' —13th Understanding, 7th Book of Sorrow",
	});

	EXPECT_STREQ(
	    R"({"errors":["\"Feed the Eidolon Ally, and it will be transformed.\" —Eris Morn","'To rend one's enemies is to see them not as equals, but objects—hollow of spirit and meaning.' —13th Understanding, 7th Book of Sorrow"]})",
	    sql.c_str()
	);
}

GTEST_TEST(get_value, does_get_value_from_json)
{
	const nlohmann::json json {
	    { "name", "Dino Spomoni"},
	    {"hours",            123},
	};

	EXPECT_STREQ(
	    "Dino Spomoni",
	    utils::get_value("name", sqlw::Type::SQL_TEXT, json).c_str()
	);

	EXPECT_STREQ(
	    "123",
	    utils::get_value("hours", sqlw::Type::SQL_TEXT, json).c_str()
	);

	EXPECT_STREQ(
	    "",
	    utils::get_value(
	        "name",
	        sqlw::Type::SQL_TEXT,
	        nlohmann::json {R"({"name": null})"}
	    )
	        .c_str()
	);

	EXPECT_STREQ(
	    "",
	    utils::get_value("name", sqlw::Type::SQL_TEXT, nlohmann::json {R"({})"})
	        .c_str()
	);

	EXPECT_STREQ(
	    "0",
	    utils::get_value(
	        "hours",
	        sqlw::Type::SQL_INT,
	        nlohmann::json {R"({"hours": null})"}
	    )
	        .c_str()
	);

	EXPECT_STREQ(
	    "0",
	    utils::get_value("hours", sqlw::Type::SQL_INT, nlohmann::json {R"({})"})
	        .c_str()
	);

	EXPECT_STREQ(
	    "0.0",
	    utils::get_value(
	        "hours",
	        sqlw::Type::SQL_DOUBLE,
	        nlohmann::json {R"({"hours": null})"}
	    )
	        .c_str()
	);

	EXPECT_STREQ(
	    "0.0",
	    utils::get_value(
	        "hours",
	        sqlw::Type::SQL_DOUBLE,
	        nlohmann::json {R"({})"}
	    )
	        .c_str()
	);
}

GTEST_TEST(get_constraint_errors, does_return_vector_with_error_messages)
{
	const nlohmann::json request_body {
	    {"a_string",            ""},
	    { "a_float", "not a float"},
	    {  "an_int",  "not an int"},
	};

	const auto errors = utils::get_constraint_errors<TestTable>(request_body);

	EXPECT_EQ(3, errors.size());
	EXPECT_STREQ("Поле a_string не должно быть пустым", errors.at(0).c_str());
	EXPECT_STREQ(
	    "Поле a_float должно быть числом с плавающей запятой",
	    errors.at(1).c_str()
	);
	EXPECT_STREQ("Поле an_int должно быть целым числом", errors.at(2).c_str());
}
