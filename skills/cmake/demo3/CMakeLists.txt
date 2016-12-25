cmake_minimum_required(VERSION 3.0)
project(DEMO3)

aux_source_directory(. SOURCES)
add_subdirectory(math)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -std=c++11")

add_executable(demo ${SOURCES})
target_link_libraries(demo math)
