cmake_minimum_required(VERSION 3.0)
project(ranked)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

if (MSVC)
    add_definitions( "/wd4244 /wd4715 /wd4474 /wd4293" )
endif(MSVC)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS -pthread)
add_compile_options(-pthread)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory (test)
add_subdirectory (util)