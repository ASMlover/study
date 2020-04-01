# Copyright (c) 2020 ASMlover. All rights reserved.
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

cmake_minimum_required(VERSION 3.7)
project(ae C)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "`${PROJECT_NAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (NOT CMAKE_C_COMPILER_ID MATCHES "GNU")
  message(FATAL_ERROR "`${PROJECT_NAME}` please use gcc in Linux")
  return()
else()
  add_definitions(
    -Wno-unused-function
    -Wno-implicit-function-declaration
    )

  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c11 -Wall -Werror -march=native")
  set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -O0")
  set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3")
endif()

file(GLOB_RECURSE LIBAE_HEADERS *.h)
file(GLOB_RECURSE LIBAE_SOURCES *.c)
message(STATUS "`${PROJECT_NAME}` LIBAE_HEADERS: ${LIBAE_HEADERS}")
message(STATUS "`${PROJECT_NAME}` LIBAE_SOURCES: ${LIBAE_SOURCES}")

add_library(${PROJECT_NAME} STATIC ${LIBAE_HEADERS} ${LIBAE_SOURCES})
