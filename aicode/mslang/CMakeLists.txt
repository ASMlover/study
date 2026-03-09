# Copyright (c) 2026 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materialsprovided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.22)
project(mslang CXX)

# -----------------------------------------------------------------------------
# Debug option switches
# -----------------------------------------------------------------------------
option(MAPLE_DEBUG_TRACE     "Trace VM execution"          OFF)
option(MAPLE_DEBUG_PRINT     "Print compiled bytecode"     OFF)
option(MAPLE_DEBUG_STRESS_GC "GC on every allocation"      OFF)
option(MAPLE_DEBUG_LOG_GC    "Log GC activity"             OFF)
option(MAPLE_NAN_BOXING      "Use NaN-boxing for Value"    ON)

# -----------------------------------------------------------------------------
# Compiler flags per toolchain
# -----------------------------------------------------------------------------
if(MSVC)
  add_compile_options(
    /std:c++latest
    /W3
    /EHsc
    /GF
    /GS
    /Gs
    /Zi
  )
  # Debug / Release differentiation via generator expressions
  add_compile_options(
    $<$<CONFIG:Debug>:/Od>
    $<$<CONFIG:Debug>:/MDd>
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Release>:/MD>
    $<$<CONFIG:Release>:/DNDEBUG>
  )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(
    -std=c++2b
    -Wall
    -Werror
    -march=native
  )
  add_compile_options(
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-DNDEBUG>
  )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(
    -std=c++2b
    -Wall
    -Werror
    -march=native
    -stdlib=libc++
  )
  add_link_options(-stdlib=libc++)
  add_compile_options(
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-DNDEBUG>
  )
endif()

# Default to Debug if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# -----------------------------------------------------------------------------
# Collect sources
# -----------------------------------------------------------------------------
file(GLOB_RECURSE MSLANG_HEADERS src/*.hh)
file(GLOB_RECURSE MSLANG_SOURCES src/*.cc)

# -----------------------------------------------------------------------------
# MSVC source_group by directory
# -----------------------------------------------------------------------------
if(MSVC)
  macro(source_group_by_dir src_files)
    foreach(file ${${src_files}})
      file(RELATIVE_PATH rel_file "${CMAKE_CURRENT_SOURCE_DIR}/src" "${file}")
      get_filename_component(dir "${rel_file}" DIRECTORY)
      if(dir)
        string(REPLACE "/" "\\\\" group_name "${dir}")
      else()
        set(group_name "\\")
      endif()
      source_group("${group_name}" FILES "${file}")
    endforeach()
  endmacro()

  source_group_by_dir(MSLANG_HEADERS)
  source_group_by_dir(MSLANG_SOURCES)
endif()

# -----------------------------------------------------------------------------
# Executable target
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME}
  ${MSLANG_HEADERS}
  ${MSLANG_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# -----------------------------------------------------------------------------
# Propagate debug options as compile definitions
# -----------------------------------------------------------------------------
if(MAPLE_DEBUG_TRACE)
  target_compile_definitions(${PROJECT_NAME} PRIVATE MAPLE_DEBUG_TRACE)
endif()
if(MAPLE_DEBUG_PRINT)
  target_compile_definitions(${PROJECT_NAME} PRIVATE MAPLE_DEBUG_PRINT)
endif()
if(MAPLE_DEBUG_STRESS_GC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE MAPLE_DEBUG_STRESS_GC)
endif()
if(MAPLE_DEBUG_LOG_GC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE MAPLE_DEBUG_LOG_GC)
endif()
if(MAPLE_NAN_BOXING)
  target_compile_definitions(${PROJECT_NAME} PRIVATE MAPLE_NAN_BOXING)
endif()

# -----------------------------------------------------------------------------
# CTest integration — iterate tests/*.ms
# -----------------------------------------------------------------------------
enable_testing()

file(GLOB MSLANG_TEST_SCRIPTS tests/*.ms)
# Tests that expect a runtime error (non-zero exit)
set(MSLANG_RUNTIME_ERROR_TESTS stack_overflow)

foreach(test_script ${MSLANG_TEST_SCRIPTS})
  get_filename_component(test_name "${test_script}" NAME_WE)
  add_test(
    NAME "test_${test_name}"
    COMMAND $<TARGET_FILE:${PROJECT_NAME}> "${test_script}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  if(test_name IN_LIST MSLANG_RUNTIME_ERROR_TESTS)
    set_tests_properties("test_${test_name}" PROPERTIES WILL_FAIL TRUE)
  endif()
endforeach()
