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
set(MINIRPC_PROJ "MiniRPC")
project(${MINIRPC_PROJ} CXX)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "`${MINIRPC_PROJ}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(MINIRPC_BUILD_DEMO "building demo of `${MINIRPC_PROJ}`" ON)

# set cxx_flags
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_definitions(
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "/W3 /GF /GS /Gs /Zi /EHsc")
else()
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g -O2 -Wall -std=c++1y")
endif()
message(STATUS "`${MINIRPC_PROJ}` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# set include_directories and link_directories
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  include_directories("D:/Tools/boost_1_63_0" "D:/Tools/protobuf/src")
  link_directories("D:/Tools/boost_1_63_0/stage/lib" "D:/Tools/protobuf/cmake/cmake-build")
else()
  include_directories(
    ${PROJECT_SOURCE_DIR}/../../../../../../OpenLib/boost_1_63_0
    ${PROJECT_SOURCE_DIR}/../../../../../../OpenLib/protobuf/src)
  link_directories(
    ${PROJECT_SOURCE_DIR}/../../../../../../OpenLib/boost_1_63_0/stage/lib
    ${PROJECT_SOURCE_DIR}/../../../../../../OpenLib/protobuf/cmake/cmake-build)
endif()

add_subdirectory(minirpc)
if (MINIRPC_BUILD_DEMO)
  add_subdirectory(demo)
endif()
