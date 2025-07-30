# Copyright (c) 2024 ASMlover. All rights reserved.
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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.22)
project(boostpp CXX)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type [Debug|Release|RelWithDebInfo|MinSizeRel]" FORCE)
endif()
message(STATUS "`${PROJECT_NAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_definitions(
    /DUNICODE
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
    /D_WINSOCK_DEPRECATED_NO_WARNINGS
    /D_WIN32_WINNT=0x0601
    /DBOOST_ASIO_DISABLE_CONCEPTS
    /D_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS
    /wd4819
    /wd4390)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /W3 /GF /GS /Gs /Zi /EHsc /std:c++latest /await")
  set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MD /Ox")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++2a -static-libgcc -static-libstdc++ -march=native")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
endif()
string(TOUPPER CMAKE_BUILD_TYPE BUILD_TYPE)
message(STATUS "`${PROJECT_NAME}` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/../../3rdparty/boost/include)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    link_directories(${PROJECT_SOURCE_DIR}/../../3rdparty/boost/lib/windows/debug)
  else()
    link_directories(${PROJECT_SOURCE_DIR}/../../3rdparty/boost/lib/windows/release)
  endif()
else()
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    link_directories(${PROJECT_SOURCE_DIR}/../../3rdparty/boost/lib/linux/debug)
  else()
    link_directories(${PROJECT_SOURCE_DIR}/../../3rdparty/boost/lib/linux/release)
  endif()
endif()

file(GLOB_RECURSE BOOSTPP_HEADERS *.hh)
file(GLOB_RECURSE BOOSTPP_SOURCES *.cc)
message(STATUS "`PROJECT_NAME` BOOSTPP_HEADERS: ${BOOSTPP_HEADERS}")
message(STATUS "`PROJECT_NAME` BOOSTPP_SOURCES: ${BOOSTPP_SOURCES}")

add_executable(${PROJECT_NAME} ${BOOSTPP_HEADERS} ${BOOSTPP_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_link_libraries(${PROJECT_NAME}
      libboost_context-vc142-mt-gd-x64-1_72.lib
      libboost_system-vc142-mt-gd-x64-1_72.lib
      libboost_regex-vc142-mt-gd-x64-1_72.lib)
  else()
    target_link_libraries(${PROJECT_NAME}
      libboost_context-vc142-mt-x64-1_72.lib
      libboost_system-vc142-mt-x64-1_72.lib
      libboost_regex-vc142-mt-x64-1_72.lib)
  endif()
else()
  target_link_libraries(${PROJECT_NAME} pthread boost_context boost_system boost_regex)
endif()
