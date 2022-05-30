cmake_minimum_required(VERSION 3.20)

project(culinary_api VERSION 0.0.1 LANGUAGES CXX)

add_executable(
	culinary_api
	src/main.cpp
	src/windows/socket.cpp
	# src/socket.cpp
	src/response.cpp
	src/request.cpp
)

if (WIN32)
	target_link_libraries(culinary_api wsock32 ws2_32)
endif()

# add_compile_options(-fconcepts-diagnostics-depth=2)

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
