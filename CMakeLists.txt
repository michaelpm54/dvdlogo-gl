cmake_minimum_required(VERSION 3.14)
project(dvd LANGUAGES CXX)

include(cmake/depends.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES
	src/main.cpp
	src/util.cpp
	lib/glad/glad.cpp
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

target_link_libraries(dvd glm SDL2)

add_custom_target(copy-runtime-files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:dvd>
    DEPENDS dvd)
