cmake_minimum_required(VERSION 3.20)

project(culinary_api VERSION 0.0.1 LANGUAGES CXX)

add_executable(
	culinary_api
	src/socket.cpp
	src/route_map.cpp
	src/main.cpp
	src/utils.cpp
	src/request_line_parser.cpp
	src/headers_parser.cpp
	src/routes/index.cpp
)

if (WIN32)
    target_link_libraries(culinary_api wsock32 ws2_32)
endif()

set_property(TARGET culinary_api PROPERTY CXX_STANDARD 20)

set(RAPIDJSON_BUILD_DOC OFF)
set(RAPIDJSON_BUILD_EXAMPLES OFF)
set(RAPIDJSON_BUILD_TESTS OFF)
set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF)

add_subdirectory(vendor/rapidjson)
add_subdirectory(lib/sql)

configure_file(
	${PROJECT_SOURCE_DIR}/include/local/config.h.in
	${PROJECT_SOURCE_DIR}/include/local/config.h
)

target_include_directories(
	culinary_api PUBLIC
	${PROJECT_SOURCE_DIR}/include/local
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/vendor/rapidjson/include
)
