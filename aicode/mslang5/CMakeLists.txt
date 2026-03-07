cmake_minimum_required(VERSION 3.25)

project(Maple LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MAPLE_BUILD_TESTS "Build Maple tests" ON)

add_library(maple_core
  src/runtime/core.cc
)

target_include_directories(maple_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
  target_compile_options(maple_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(maple_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(maple_cli
  src/main.cc
  src/cli/app.cc
)

target_link_libraries(maple_cli PRIVATE maple_core)

target_include_directories(maple_cli
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
  target_compile_options(maple_cli PRIVATE /W4 /permissive-)
else()
  target_compile_options(maple_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MAPLE_BUILD_TESTS)
  include(CTest)
endif()
