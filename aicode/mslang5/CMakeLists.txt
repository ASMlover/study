cmake_minimum_required(VERSION 3.25)

project(Maple LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MAPLE_BUILD_TESTS "Build Maple tests" ON)

add_library(maple_core
  src/runtime/core.cc
  src/support/logger.cc
  src/support/source.cc
  src/bytecode/chunk.cc
  src/bytecode/disasm.cc
  src/frontend/lexer.cc
  src/frontend/parser.cc
  src/frontend/compiler.cc
  src/runtime/object.cc
  src/runtime/table.cc
  src/runtime/gc.cc
  src/runtime/module.cc
  src/runtime/script_interpreter.cc
  src/runtime/vm.cc
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
  add_executable(maple_tests
    src/cli/app.cc
    tests/unit/test_main.cc
    tests/unit/test_logger.cc
    tests/unit/test_source.cc
    tests/unit/test_chunk_disasm.cc
    tests/unit/test_lexer.cc
    tests/unit/test_vm_compiler.cc
    tests/unit/test_module.cc
    tests/integration/test_cli.cc
    tests/integration/test_language_closure.cc
    tests/integration/test_language_class.cc
    tests/integration/test_language_resolver.cc
    tests/integration/test_conformance.cc
    tests/integration/test_diagnostics_golden.cc
  )
  target_link_libraries(maple_tests PRIVATE maple_core)
  target_include_directories(maple_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit
  )
  if(MSVC)
    target_compile_options(maple_tests PRIVATE /W4 /permissive-)
  else()
    target_compile_options(maple_tests PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  add_test(NAME maple_tests_all COMMAND maple_tests --suite=all)
  set_tests_properties(maple_tests_all PROPERTIES LABELS "all")

  add_test(NAME maple_tests_unit COMMAND maple_tests --suite=unit)
  set_tests_properties(maple_tests_unit PROPERTIES LABELS "unit")

  add_test(NAME maple_tests_integration COMMAND maple_tests --suite=integration)
  set_tests_properties(maple_tests_integration PROPERTIES LABELS "integration")

  add_test(NAME maple_tests_conformance COMMAND maple_tests --suite=conformance)
  set_tests_properties(maple_tests_conformance PROPERTIES LABELS "conformance")

  add_test(NAME maple_tests_diagnostics COMMAND maple_tests --suite=diagnostics)
  set_tests_properties(maple_tests_diagnostics PROPERTIES LABELS "diagnostics")
endif()
