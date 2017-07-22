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
set(LEARNGC_PROJECT "LearnGC")
project(${LEARNGC_PROJECT} CXX)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "[${LEARNGC_PROJECT}] CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if (MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "you need Visual Studio 2015 or later")
    return()
  endif()

  add_definitions(
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /GF /GS /Gs /Zi /EHsc")
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

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -O2 -Wall -std=c++1y")
endif()
message(STATUS "[${LEARNGC_PROJECT}] CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR}/../../../Chaos)
link_directories(${PROJECT_SOURCE_DIR}/../../../Chaos/cmake-build/Chaos)

add_subdirectory(MarkSweep)
add_subdirectory(MarkSweep2)
add_subdirectory(TraceSweep)
add_subdirectory(RefSweep)
add_subdirectory(DelayRef)
add_subdirectory(TraceMemory)
add_subdirectory(RefMemory)
add_subdirectory(DelayMemory)
add_subdirectory(SemispaceCopy)
add_subdirectory(NurserySweep)
add_subdirectory(NurseryMemory)
add_subdirectory(SweepGC)
add_subdirectory(SweepMemory)
add_subdirectory(ParallelMark)
add_subdirectory(ParallelMarkMemory)
add_subdirectory(ParallelSweep)
add_subdirectory(ParallelSweepMemory)
add_subdirectory(ParallelGC)
add_subdirectory(ParallelGCMemory)
add_subdirectory(ParallelChannel)
add_subdirectory(ParallelChannelMemory)
