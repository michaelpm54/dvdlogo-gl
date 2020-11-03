cmake_minimum_required(VERSION 3.14)
project(dvd LANGUAGES CXX)

if (UNIX)
	option(FETCH_LIBS "Download dependencies automatically" OFF)
else()
	option(FETCH_LIBS "Download dependencies automatically" ON)
endif()

if (FETCH_LIBS)
	include(cmake/depends.cmake)
endif()

set(SOURCES
	src/main.cpp
	src/util.cpp
	lib/glew.cpp
)

add_executable(dvd
	${SOURCES}
)

target_include_directories(dvd
	PRIVATE
		src
		include
		${FETCHCONTENT_BASE_DIR}/sdl2-src/include
)

if (UNIX)
	find_package(glm CONFIG REQUIRED)
	find_package(SDL2 CONFIG REQUIRED)
	target_link_libraries(dvd dl glm SDL2 GL)
else ()
	target_link_libraries(dvd glm SDL2 opengl32)
endif()

add_custom_target(copy-runtime-files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:dvd>
    DEPENDS dvd)
