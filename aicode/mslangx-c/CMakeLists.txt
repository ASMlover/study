cmake_minimum_required(VERSION 3.20)

project(mslangc VERSION 0.1.0 LANGUAGES C)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(MslangcTesting)

add_executable(mslangc
  src/main.c)

target_compile_features(mslangc PRIVATE c_std_11)

if(MSVC)
  target_compile_options(mslangc PRIVATE /W4 /WX)
else()
  target_compile_options(mslangc PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_custom_target(check
  COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C $<CONFIG>
  DEPENDS mslangc
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Running mslangc test suite")

add_subdirectory(tests)