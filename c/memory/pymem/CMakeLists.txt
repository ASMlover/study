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
project(PyMem CXX C)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "[PyMem] CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(PYMEM_USE_MMAP "use mmap in linux ?" OFF)
option(PYMEM_PYTHON "use python memory allocating ?" OFF)
option(PYMEM_USE_TCMALLOC "use tcmalloc for linux ?" OFF)
option(PYMEM_USE_JEMALLOC "use jemalloc for linux ?" ON)

if (PYMEM_PYTHON)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPYMEM_PYTHON")
endif()
if (PYMEM_USE_JEMALLOC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPYMEM_USE_JEMALLOC")
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_definitions(
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /GF /GS /Gs /Zi")
  set(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MDd")
  set(CMAKE_C_FLAGS_RELEASE "/DNDEBUG /Ox /MD")
else()
  if (PYMEM_USE_MMAP)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall -DPYMEM_USE_MMAP -march=native -fno-strict-aliasing")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -march=native -fno-strict-aliasing")
  endif()
  set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -O0")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -fwrapv -Wstrict-prototypes")
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "[PyMem] CMAKE_C_FLAGS: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BUILD_TYPE}}")

if (PYMEM_USE_JEMALLOC)
  include_directories(/home/hzhukerong/OpenLib/jemalloc-5.0.1/include)
  link_directories(/home/hzhukerong/OpenLib/jemalloc-5.0.1/lib)
endif()

aux_source_directory(. PYMEM_SOURCES)
message(STATUS "[PyMem] PYMEM_SOURCES: ${PYMEM_SOURCES}")

add_executable(pymem ${PYMEM_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  if (PYMEM_USE_JEMALLOC)
    target_link_libraries(pymem -ljemalloc)
  elseif (PYMEM_USE_TCMALLOC)
    target_link_libraries(pymem -ltcmalloc_minimal)
  endif()
endif()
