cmake_minimum_required(VERSION 3.20)

project(mslangc VERSION 0.1.0 LANGUAGES C)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(MslangcTesting)

add_executable(mslangc
  src/main.c)

add_library(mslang_support
  src/support/arena.c
  src/support/buffer.c
  src/support/diag.c)

add_library(mslang_runtime_core
  src/runtime/object.c
  src/runtime/string.c
  src/runtime/table.c
  src/runtime/value.c)

add_library(mslang_frontend
  src/frontend/ast.c
  src/frontend/lexer.c
  src/frontend/parser_expr.c)

target_compile_features(mslangc PRIVATE c_std_11)
target_compile_features(mslang_support PUBLIC c_std_11)
target_compile_features(mslang_runtime_core PUBLIC c_std_11)
target_compile_features(mslang_frontend PUBLIC c_std_11)

target_include_directories(mslangc PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(mslang_support PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(mslang_runtime_core PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(mslang_frontend PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(mslang_runtime_core PUBLIC mslang_support)
target_link_libraries(mslang_frontend PUBLIC mslang_support)

if(MSVC)
  target_compile_options(mslangc PRIVATE /W4 /WX)
  target_compile_options(mslang_support PRIVATE /W4 /WX)
  target_compile_options(mslang_runtime_core PRIVATE /W4 /WX)
  target_compile_options(mslang_frontend PRIVATE /W4 /WX)
else()
  target_compile_options(mslangc PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(mslang_support PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(mslang_runtime_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(mslang_frontend PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_custom_target(check
  COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C $<CONFIG>
  DEPENDS mslangc
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Running mslangc test suite")

add_subdirectory(tests)

add_dependencies(check
  support.diag
  support.arena
  support.buffer
  runtime_core.value
  runtime_core.string
  runtime_core.table
  lexer.unit
  lexer.fixture
  parser_expr.unit
  parser_expr.fixture)