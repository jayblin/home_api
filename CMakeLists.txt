cmake_minimum_required(VERSION 3.20)

project(culinary_api VERSION 0.0.1 LANGUAGES CXX)

add_executable(
	culinary_api
	src/main.cpp
	src/socket.cpp
	src/response.cpp
	src/request.cpp
)

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
