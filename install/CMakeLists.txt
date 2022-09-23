cmake_minimum_required(VERSION 3.23)

project(culinary_api_install VERSION 0.0.1 LANGUAGES CXX)

add_executable(culinary_api_install install.cpp)

add_dependencies(culinary_api_install sql)

set_property(TARGET culinary_api_install PROPERTY CXX_STANDARD 20)

target_include_directories(
	culinary_api_install
	PUBLIC ${CMAKE_SOURCE_DIR}/lib/sql/include
)

target_link_libraries(
	culinary_api_install
	PUBLIC sql
)

configure_file(
	${PROJECT_SOURCE_DIR}/config.h.in
	${PROJECT_SOURCE_DIR}/config.h
)

install(
	TARGETS culinary_api_install sql
	RUNTIME DESTINATION bin
)
