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
project(DevilPROJ CXX C)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "`DevilPROJ` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(DEVIL_BUILD_TEST "build `DevilPROJ` examples and unittests" ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
      message(FATAL_ERROR "you need Gcc-4.8.1 or later")
      return()
    endif()
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.3")
      message(FATAL_ERROR "you need Clang-3.3 or later")
      return()
    endif()
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -std=c11 -Wall -Werror")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if (MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "you need Visual Studio 2015 or later")
    return()
  endif()
  add_definitions(
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W3 -GF -GS -Gs -Zi")
endif()
message(STATUS "`DevilPROJ` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

add_subdirectory(src)
if (DEVIL_BUILD_TEST)
  add_subdirectory(test)
endif()
