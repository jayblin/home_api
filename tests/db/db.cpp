/* #include "util/db.hpp" */
#include "constraint/constraint.hpp"
#include "sqlw/forward.hpp"
#include <gtest/gtest.h>

/* struct TestTable */
/* { */
/* 	using C = constraint::Constraint; */

/* 	static constexpr std::string_view table_name = "test_table"; */

/* 	static constexpr std::tuple< */
/* 	    std::tuple<constraint::column_name_t, sqlw::Type, std::array<C, 3>>, */
/* 	    std::tuple<constraint::column_name_t, sqlw::Type, std::array<C, 2>>, */
/* 	    std::tuple<constraint::column_name_t, sqlw::Type>, */
/* 	    std::tuple<constraint::column_name_t, sqlw::Type>> */
/* 	    columns { */
/* 			{"id", sqlw::Type::SQL_INT, {C::PRIMARY, C::KEY, C::AUTOINCREMENT}}, */
/* 	        {"a_string", sqlw::Type::SQL_TEXT, {C::NOT_NULL, C::UNIQUE}}, */
/* 	        {"a_float", sqlw::Type::SQL_DOUBLE}, */
/* 	        {"an_int", sqlw::Type::SQL_INT}, */
/* 			{"hello"}, */
/*     }; */
/* }; */

/* GTEST_TEST(generate_insert_clause, generates_correct_clause) */
/* { */
/* 	std::string sql = util::db::generate_insert_clause<TestTable>(); */

/* 	EXPECT_STREQ( */
/* 	    "INSERT INTO test_table (a_string,a_float,an_int) VALUES (?,?,?)", */
/* 	    sql.c_str() */
/* 	); */
/* } */

/* GTEST_TEST(generate_select_last_insert, generates_correct_clause) */
/* { */
/* 	std::string sql = */
/* 	    util::db::generate_select_last_insert(TestTable::table_name); */

/* 	EXPECT_STREQ( */
/* 	    "SELECT e0.* FROM test_table e0 WHERE e0.id = last_insert_rowid()", */
/* 	    sql.c_str() */
/* 	); */
/* } */

/* GTEST_TEST(generate_select_last_insert, provides_expected_results_on_exec) */
/* { */
/* 	sqlw::Connection con {":memory:"}; */
/* 	sqlw::Statement stmt {&con}; */

/* 	stmt("CREATE TABLE test_table"); */
/* 	stmt(R"(CREATE TABLE test_table ( */
/* 		id INTEGER PRIMARY KEY AUTOINCREMENT, */
/* 		name TEXT NOT NULL UNIQUE */
/* 	);)"); */
/* 	stmt("INSERT INTO test_table (id, name) VALUES (1,'kate');"); */

/* 	std::string sql = */
/* 	    util::db::generate_select_last_insert(TestTable::table_name); */

/* 	std::stringstream ss; */
/* 	stmt( */
/* 	    sql, */
/* 	    [&ss](sqlw::Statement::ExecArgs args) */
/* 	    { */
/* 		    ss << args.column_value << ','; */
/* 	    } */
/* 	); */

/* 	EXPECT_STREQ("1,kate,", ss.str().c_str()); */
/* } */

/* GTEST_TEST(idk, idk) */
/* { */
/* 	sqlw::Connection con {":memory:"}; */

/* 	util::db::create_table_from_constraint<TestTable>(con); */

/* 	sqlw::Statement stmt {&con}; */

/* 	stmt.prepare( */
/* 	    "SELECT name FROM sqlite_master WHERE type='table' AND name=?" */
/* 	); */
/* 	stmt.bind(1, TestTable::table_name, sqlw::Type::SQL_TEXT); */
/* 	bool table_exists = false; */
/* 	stmt( */
/* 	    [&table_exists](sqlw::Statement::ExecArgs args) */
/* 	    { */
/* 			std::cout << args.column_count << '\n'; */
/* 			std::cout << args.column_name << '\n'; */
/* 			/1* std::cout << args.column_type << '\n'; *1/ */
/* 			std::cout << args.column_value << '\n'; */
/* 		    table_exists = true; */
/* 	    } */
/* 	); */

/* 	EXPECT_TRUE(table_exists) */
/* 	    << "Table " << TestTable::table_name << " must must be created"; */
/* } */
