# Copyright (c) 2021 ASMlover. All rights reserved.
#
#  _____         _             _
# |_   _|_ _  __| |_ __   ___ | | ___
#   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
#   | | (_| | (_| | |_) | (_) | |  __/
#   |_|\__,_|\__,_| .__/ \___/|_|\___|
#                 |_|
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
project(TadpolePROJ CXX)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "`${PROJECT_NAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(TADPOLE_RUN_HARNESS "Enable/Disable run test harness" OFF)
option(TADPOLE_DEBUG_GC "Enable/Disable gc debug information" OFF)
option(TADPOLE_DEBUG_VM "Enable/Disable vm debug information" OFF)
option(TADPOLE_BUILD_REFER "Enable/Disable build `refer`" OFF)

if (TADPOLE_RUN_HARNESS)
  add_compile_options(-D_TADPOLE_RUN_HARNESS)
endif()
if (TADPOLE_DEBUG_GC)
  add_compile_options(-D_TADPOLE_DEBUG_GC)
endif()
if (TADPOLE_DEBUG_VM)
  add_compile_options(-D_TADPOLE_DEBUG_VM)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_definitions(
    /DUNICODE
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /GF /GS /Gs /Zi /EHsc /std:c++latest /await")
  set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MD /Ox")
else()
  add_definitions(-Wno-switch)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Wall -Werror -march=native")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions(-Wno-unused-private-field)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  else()
    add_definitions(-Wno-format-truncation -Wno-comment)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
  endif()
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "`${PROJECT_NAME}` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")

macro(source_group_by_dir source_files)
  # only for MSVC
  set(current_dir ${CMAKE_CURRENT_SOURCE_DIR})
  foreach(fname ${${source_files}})
    string(REGEX REPLACE ${current_dir}/\(.*\) \\1 fpath ${fname})
    string(REGEX REPLACE "\(.*\)/.*" \\1 group_name ${fpath})
    string(COMPARE EQUAL ${fpath} ${group_name} nogroup)
    string(REPLACE "/" "\\" group_name ${group_name})
    if (nogroup)
      set(group_name "\\")
    endif()
    source_group(${group_name} FILES ${fname})
  endforeach()
endmacro()

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(tadpole)
if (TADPOLE_BUILD_REFER)
  add_subdirectory(refer/mark_sweep)
  add_subdirectory(refer/semispace_copy)
  add_subdirectory(refer/semispace_copy2)
  add_subdirectory(refer/reference_count)
  add_subdirectory(refer/gc)
endif()
