cmake_minimum_required(VERSION 3.0)
project(DEMO2)

aux_source_directory(. SOURCES)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -std=c++11")

add_executable(demo ${SOURCES})
