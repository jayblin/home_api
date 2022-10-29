cmake_minimum_required(VERSION 3.23)

project(culinary_api VERSION 0.0.1 LANGUAGES CXX)

add_executable(
	culinary_api
	src/socket.cpp
	src/route_map.cpp
	src/main.cpp
	src/utils.cpp
	src/status_line_parser.cpp
	src/headers_parser.cpp
	src/routes/index.cpp
	src/request_parser.cpp
	src/parsor.cpp
)

if (WIN32)
	list(APPEND EXTRA_LIBS wsock32)
	list(APPEND EXTRA_LIBS ws2_32)
endif()

set_property(TARGET culinary_api PROPERTY CXX_STANDARD 20)

add_subdirectory(lib/sql)
add_subdirectory(install)

add_dependencies(culinary_api culinary_api_install)

configure_file(
	${PROJECT_SOURCE_DIR}/include/local/config.h.in
	${PROJECT_SOURCE_DIR}/include/local/config.h
)

target_include_directories(
	culinary_api
	PUBLIC
		${PROJECT_SOURCE_DIR}/include/local
		${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
	culinary_api
	PUBLIC
		sql
		${EXTRA_LIBS}
)

install(
	TARGETS culinary_api
	RUNTIME DESTINATION bin
)

# GoogleTest

# https://google.github.io/googletest/quickstart-cmake.html
include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_custom_target(tests)

add_executable(
	tests_executable
	src/parsor.cpp
	tests/parsor.cpp
	src/status_line_parser.cpp
	tests/status_line_parser.cpp
	src/headers_parser.cpp
	tests/headers_parser.cpp
	src/request_parser.cpp
	tests/request_parser.cpp
)

target_link_libraries(
	tests_executable
	GTest::gtest_main
)

set_property(TARGET tests_executable PROPERTY CXX_STANDARD 20)

target_include_directories(
	tests_executable
	PUBLIC
	${CMAKE_SOURCE_DIR}/include/local
	${CMAKE_SOURCE_DIR}/include
)

add_dependencies(tests tests_executable)

include(GoogleTest)
gtest_discover_tests(tests_executable)

#~GoogleTest
