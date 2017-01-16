# Copyright (c) 2017 ASMlover. All rights reserved.
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

cmake_minimum_required(VERSION 3.0)
project(NeptunePROJ CXX)

if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

# set NeptunePROJ version
set(NEPTUNE_MAJOR_VERSION "0")
set(NEPTUNE_MINOR_VERSION "1")
set(NEPTUNE_PATCH_VERSION "0")
set(NEPTUNE_VERSION_STRING "${NEPTUNE_MAJOR_VERSION}.${NEPTUNE_MINOR_VERSION}.${NEPTUNE_PATCH_VERSION}")

# compile in release with debug info mode by default
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "choose build type [Debug|Release|RelWithDebInfo|MinSizeRel]" FORCE)
endif()
message(STATUS "`NeptunePROJ` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# build all binaries in a separate directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set building options
option(NEPTUNE_BUILD_TEST "build Neptune examples and unittests" ON)

# set building CMAKE_CXX_FLAGS
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if (MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "you need Visual Studio 2015 or later")
    return()
  endif()

  add_definitions(
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
    /wd4091
    /wd4244
    /wd4800
    /wd4996)
  set(CMAKE_CXX_FLAGS "/W3 /GF /GS /Gs /Zi /EHsc")
else()
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
      message(FATAL_ERROR "you need GCC-4.8.1 or later")
      return()
    endif()
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.3")
      message(FATAL_ERROR "you need Clang-3.3 or later")
      return()
    endif()
  endif()

  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g -O2 -std=c++11 -Wall -Werror")
endif()
message(STATUS "`NeptunePROJ` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../../../Chaos)
add_subdirectory(Neptune)
if (NEPTUNE_BUILD_TEST)
  # add_subdirectory(Tests)
endif()
