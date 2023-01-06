cmake_minimum_required(VERSION 3.23)

project(sql VERSION 0.0.1 LANGUAGES CXX C)

add_library(
	sql
	SHARED
		${CMAKE_SOURCE_DIR}/vendor/sqlite/sqlite3.c
		sqlite3.cpp
		statement.cpp
		status.cpp
		json_string_result.cpp
)

set_property(TARGET sql PROPERTY CXX_STANDARD 20)

target_include_directories(
	sql
	PUBLIC
		${CMAKE_SOURCE_DIR}/vendor/sqlite/include
		include
)
