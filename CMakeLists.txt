cmake_minimum_required(VERSION 3.23)

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
