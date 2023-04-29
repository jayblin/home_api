cmake_minimum_required(VERSION 3.23)

project(home_api_install VERSION 0.0.1 LANGUAGES CXX)

add_executable(home_api_install install.cpp)

add_dependencies(home_api_install sqlw)

set_property(TARGET home_api_install PROPERTY CXX_STANDARD 20)

# target_include_directories(
# 	home_api_install
# 	PUBLIC ${CMAKE_SOURCE_DIR}/lib/sql/include
# )

target_link_libraries(
	home_api_install
	PUBLIC sqlw
)

configure_file(
	${PROJECT_SOURCE_DIR}/config.h.in
	${PROJECT_SOURCE_DIR}/config.h
)

install(
	TARGETS home_api_install sqlw
	RUNTIME DESTINATION bin
)
