cmake_minimum_required(VERSION 3.20)

project(culinary_api VERSION 0.0.1 LANGUAGES CXX)

add_executable(
	culinary_api
	src/socket.cpp
	src/request.cpp
	src/route-map.cpp
	src/main.cpp
)

if (WIN32)
    target_link_libraries(culinary_api wsock32 ws2_32)
endif()

set_property(TARGET culinary_api PROPERTY CXX_STANDARD 20)

configure_file(
	${PROJECT_SOURCE_DIR}/include/local/config.h.in
	${PROJECT_SOURCE_DIR}/include/local/config.h
)

target_include_directories(
	culinary_api PUBLIC
	${PROJECT_SOURCE_DIR}/include/local
	${PROJECT_SOURCE_DIR}/include
)
