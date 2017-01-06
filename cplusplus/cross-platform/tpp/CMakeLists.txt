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
project(TppPROJ CXX)

if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if (MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "you need Visual Studio 2015 or later")
    return()
  endif()

  add_definitions(
    /D_UNICODE
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

aux_source_directory(. ALL_SOURCES)

file(GLOB_RECURSE BASIC_SOURCES basic/*.cc)
file(GLOB_RECURSE CO_SOURCES co/*.cc)
set(ALL_SOURCES ${ALL_SOURCES} ${BASIC_SOURCES} ${CO_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  file(GLOB_RECURSE WINDOWS_SOURCES windows/*.cc)
  set(ALL_SOURCES ${ALL_SOURCES} ${WINDOWS_SOURCES})
else()
  file(GLOB_RECURSE POSIX_SOURCES posix/*.cc)
  set(ALL_SOURCES ${ALL_SOURCES} ${POSIX_SOURCES})
  if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    file(GLOB_RECURSE LINUX_SOURCES linux/*.cc)
    set(ALL_SOURCES ${ALL_SOURCES} ${LINUX_SOURCES})
  elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    file(GLOB_RECURSE DARWIN_SOURCES darwin/*.cc)
    set(ALL_SOURCES ${ALL_SOURCES} ${DARWIN_SOURCES})
  endif()
endif()
message(STATUS "ALL_SOURCES: ${ALL_SOURCES}")

add_executable(tpp.test ${ALL_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(tpp.test winmm.lib Dbghelp.lib)
else()
  target_link_libraries(tpp.test pthread)
endif()
