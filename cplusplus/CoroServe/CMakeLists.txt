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

cmake_minimum_required(VERSION 3.14)
project(CoroServe CXX)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "`${PROJECT_NAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(USE_LIBUV_STATIC "use static libuv library" ON)
option(USE_ASIO_THREADPOOL "use threadpool based on boost.asio" OFF)

if (USE_ASIO_THREADPOOL)
  add_definitions(-DUSE_ASIO_THREADPOOL)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_definitions(
    /DUNICODE
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
    /D_WINSOCK_DEPRECATED_NO_WARNINGS
    /D_WIN32_WINNT=0x0601
    /DCORO_WIN
    /DBOOST_ASIO_DISABLE_CONCEPTS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /GF /GS /Gs /Zi /EHsc /std:c++latest /await")
  set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MD /Ox")
else()
  add_definitions(-DCORO_LINUX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++2a -static-libgcc -static-libstdc++ -march=native")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "`${PROJECT_NAME}` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/../../3rdparty/libuv/include
  ${PROJECT_SOURCE_DIR}/../../3rdparty/boost/include)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    link_directories(
      ${PROJECT_SOURCE_DIR}/../../3rdparty/libuv/lib/windows/debug
      ${PROJECT_SOURCE_DIR}/../../3rdparty/boost/lib/windows/debug)
  else()
    link_directories(
      ${PROJECT_SOURCE_DIR}/../../3rdparty/libuv/lib/windows/release
      ${PROJECT_SOURCE_DIR}/../../3rdparty/boost/lib/windows/release)
  endif()
else()
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    link_directories(
      ${PROJECT_SOURCE_DIR}/../../3rdparty/libuv/lib/linux/debug
      ${PROJECT_SOURCE_DIR}/../../3rdparty/boost/lib/linux/debug)
  else()
    link_directories(
      ${PROJECT_SOURCE_DIR}/../../3rdparty/libuv/lib/linux/release
      ${PROJECT_SOURCE_DIR}/../../3rdparty/boost/lib/linux/release)
  endif()
endif()

file(GLOB_RECURSE COROSERVER_HEADERS *.hh)
file(GLOB_RECURSE COROSERVER_SOURCES *.cc)
source_group("Header Files" FILES ${COROSERVER_HEADERS})
source_group("Source Files" FILES ${COROSERVER_SOURCES})
message(STATUS "`${PROJECT_NAME}` COROSERVER PROJECT FILES: ${COROSERVER_HEADERS} ${COROSERVER_SOURCES}")

add_executable(${PROJECT_NAME} ${COROSERVER_HEADERS} ${COROSERVER_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  if (USE_LIBUV_STATIC)
    target_link_libraries(${PROJECT_NAME} ws2_32.lib iphlpapi.lib psapi.lib userenv.lib uv_a.lib)
  else()
    target_link_libraries(${PROJECT_NAME} ws2_32.lib uv.lib)
  endif()
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_link_libraries(${PROJECT_NAME}
      libboost_context-vc142-mt-gd-x64-1_72.lib libboost_system-vc142-mt-gd-x64-1_72.lib)
  else()
    target_link_libraries(${PROJECT_NAME}
      libboost_context-vc142-mt-x64-1_72.lib libboost_system-vc142-mt-x64-1_72.lib)
  endif()
else()
  if (USE_LIBUV_STATIC)
    target_link_libraries(${PROJECT_NAME} uv_a)
  else()
    target_link_libraries(${PROJECT_NAME} uv)
  endif()
  target_link_libraries(${PROJECT_NAME} pthread boost_context boost_system)
endif()
