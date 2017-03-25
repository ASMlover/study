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
set(PROJ_NAME "EchoPROJ")
project(${PROJ_NAME} CXX)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "<${PROJ_NAME}> CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_definitions(
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS)

  set(CMAKE_CXX_FLAGS "/W3 /GF /GS /Gs /Zi /EHsc")

  include_directories("D:/Tools/boost_1_63_0")
  link_directories("D:/Tools/boost_1_63_0/stage/lib")
else()
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g -O2 -Wall -std=c++1y")

  include_directories(${PROJECT_SOURCE_DIR}/../../../../../Opensource/boost_1_63_0)
  link_directories(${PROJECT_SOURCE_DIR}/../../../../../Opensource/boost_1_63_0/stage/lib)
endif()
message(STATUS "<${PROJ_NAME}> CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set(PROJ_BIN "echo")
aux_source_directory(. PROJ_SOURCES)

add_executable(${PROJ_BIN} ${PROJ_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(${PROJ_BIN}
    debug libboost_system-vc140-mt-gd-1_63 optimized libboost_system-vc140-mt-1_63)
else()
  target_link_libraries(${PROJ_BIN} boost_system pthread)
endif()
