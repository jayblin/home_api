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
	src/route_map.cpp
	# src/local/utils.cpp
	src/http/status_line_parser.cpp
	src/http/headers_parser.cpp
	src/http/request_parser.cpp
	src/http/parsor.cpp
	src/http/query_parser.cpp
	src/routes/index.cpp
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
	GIT_TAG 		7dd5b5f9b239c877dca3bb349350fdfb4b5f1f81
)

FetchContent_Declare(
  sqlw
  GIT_REPOSITORY https://github.com/jayblin/sqlw.git
  GIT_TAG        d507afbc132210fd24b0a959cd909430ee556843
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

	src/http/status_line_parser.cpp
	tests/http/status_line_parser.cpp

	src/http/request_parser.cpp
	tests/http/request_parser.cpp

	src/http/query_parser.cpp
	tests/http/query_parser.cpp

	src/http/headers_parser.cpp
	tests/http/headers_parser.cpp

	# src/local/util.cpp
	# tests/local/util.cpp
	# src/util/db.cpp
	# tests/util/db.cpp
	# src/util/http.cpp
	# src/route_map.cpp
	# src/routes/index.cpp
	# src/server/server.cpp
	# tests/main.cpp
	# src/db/db.cpp
	# tests/db/migration.cpp
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
	src/install.cpp
	# src/local/utils.cpp
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
	# TARGETS sock sqlw home_api home_api_install_executable
	# TARGETS sqlw home_api_install_executable
	TARGETS sock sqlw home_api_tests_executable
	RUNTIME DESTINATION bin
)

# ~INSTALL

configure_file(
	${PROJECT_SOURCE_DIR}/include/local/cmake_vars.h.in
	${PROJECT_SOURCE_DIR}/include/local/cmake_vars.h
)
