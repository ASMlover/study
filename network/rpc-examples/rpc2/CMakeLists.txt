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
set(RPC_PROJNAME "RpcPROJ")
project(${RPC_PROJNAME} CXX)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "`${RPC_PROJNAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_definitions(
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "/W3 /GF /GS /Gs /Zi /EHsc")

  include_directories("D:/Tools/boost_1_63_0" "D:/Tools/protobuf/src")
  link_directories("D:/Tools/boost_1_63_0/stage/lib" "D:/Tools/protobuf/cmake/cmake-build")
else()
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g -O2 -Wall -std=c++11")

  include_directories(
    ${PROJECT_SOURCE_DIR}/../../../../../../OpenLib/boost_1_63_0
    ${PROJECT_SOURCE_DIR}/../../../../../../OpenLib/protobuf/src)
  link_directories(
    ${PROJECT_SOURCE_DIR}/../../../../../../OpenLib/boost_1_63_0/stage/lib
    ${PROJECT_SOURCE_DIR}/../../../../../../OpenLib/protobuf/cmake/cmake-build)
endif()
message(STATUS "`${RPC_PROJNAME}` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set(RPC_SOURCES

  echo.pb.cc
  tcp_client.cc
  tcp_connection.cc
  tcp_server.cc)

set(RPC_LIBNAME "tinyrpc")
add_library(${RPC_LIBNAME} STATIC ${RPC_SOURCES})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(${RPC_LIBNAME}
    debug libboost_system-vc140-mt-gd-1_63 optimized libboost_system-vc140-mt-1_63
    debug libprotobufd optimized libprotobuf)
else()
  target_link_libraries(${RPC_LIBNAME} pthread boost_system protobuf)
endif()

add_executable(rpc.server server.cc)
target_link_libraries(rpc.server ${RPC_LIBNAME})

add_executable(rpc.client client.cc)
target_link_libraries(rpc.client ${RPC_LIBNAME})
