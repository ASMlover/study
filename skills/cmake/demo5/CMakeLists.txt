cmake_minimum_required(VERSION 3.0)
project(DEMO5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -std=c++11")
file(GLOB SOURCES
  "*.cc"
  "math/*.cc"
  )

add_executable(demo ${SOURCES})
