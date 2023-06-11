cmake_minimum_required(VERSION 3.23)

project(home_api VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(
	GCC_COMPILER_FLAGS
	"-g;-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused"
)

add_executable(
	home_api
	src/http/parsor.cpp
	src/util/json.cpp
	src/util/http.cpp
	src/util/base64.cpp
	src/route_map.cpp
	src/routes/index.cpp
	src/server/server.cpp
	src/http/status_line_parser.cpp
	src/http/query_parser.cpp
	src/http/headers_parser.cpp
	src/db/db.cpp
	src/main.cpp
)

target_include_directories(
	home_api
	PUBLIC
		include
)

include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)

FetchContent_Declare(
	sock
	GIT_REPOSITORY 	https://github.com/jayblin/sock.git
	GIT_TAG 		a6643947c404897fd1d6dae8131abbe364e9dd42
)

FetchContent_Declare(
  sqlw
  GIT_REPOSITORY https://github.com/jayblin/sqlw.git
  GIT_TAG        383d140bae7cc96b351fe370c9b8fdb4aee34aba
)

FetchContent_Declare(
	json
	URL
	https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)

FetchContent_MakeAvailable(sock sqlw json)

target_link_libraries(
	home_api
	PRIVATE
		sock
		nlohmann_json::nlohmann_json
		sqlw
)

# TESTS

# https://google.github.io/googletest/quickstart-cmake.html

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_custom_target(home_api_tests)

add_executable(
	home_api_tests_executable
	src/http/parsor.cpp
	src/util/json.cpp
	src/util/http.cpp
	src/util/base64.cpp

	src/route_map.cpp
	src/routes/index.cpp
	src/server/server.cpp

	src/http/status_line_parser.cpp
	tests/http/status_line_parser.cpp

	src/http/query_parser.cpp
	tests/http/query_parser.cpp

	src/http/headers_parser.cpp
	tests/http/headers_parser.cpp

	src/http/parser.cpp
	tests/http/parser.cpp

	src/db/db.cpp
	tests/db/db.cpp
	tests/db/migration.cpp

	tests/main.cpp
)

target_link_libraries(
	home_api_tests_executable
	PRIVATE
		GTest::gtest_main
		sock
		sqlw
		nlohmann_json::nlohmann_json
)

target_include_directories(
	home_api_tests_executable
	PUBLIC
		include
)

add_dependencies(home_api_tests home_api_tests_executable)

include(GoogleTest)

# gtest_discover_tests(home_api_tests_executable)

#~TESTS

# INSTALL

add_custom_target(home_api_install)

add_executable(
	home_api_install_executable
	src/db/db.cpp
	src/install.cpp
)

target_link_libraries(
	home_api_install_executable
	PRIVATE
		sock
		sqlw
		nlohmann_json::nlohmann_json
)

add_dependencies(home_api_install home_api_install_executable)

target_include_directories(
	home_api_install_executable
	PUBLIC include
)

install(
	TARGETS
		sock
		sqlw
		home_api_tests_executable
		home_api_install_executable
		home_api
	RUNTIME
	DESTINATION bin
	OPTIONAL
)

# ~INSTALL

configure_file(
	${PROJECT_SOURCE_DIR}/include/local/cmake_vars.h.in
	${PROJECT_SOURCE_DIR}/include/local/cmake_vars.h
)
