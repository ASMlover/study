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
project(selectors CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "`${PROJECT_NAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(SEL_RUN_HARNESS "enable/disable run test harness" OFF)

if (SEL_RUN_HARNESS)
    add_compile_options(-D_SEL_RUN_HARNESS)
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Werror -march=native")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_definitions(
            -Wno-unused-private-field)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else()
        add_definitions(
            -Wno-format-truncation)
    endif()
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "`${PROJECT_NAME}` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")

include_directories(${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE SEL_HEADERS *.hh)
file(GLOB_RECURSE SEL_SOURCES *.cc)
source_group("Header Files" FILES ${SEL_HEADERS})
source_group("Source Files" FILES ${SEL_SOURCES})

message(STATUS "Scanning `${PROJECT_NAME}`:")
get_filename_component(_SEL_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
foreach(_SEL_FULLPATH IN LISTS SEL_HEADERS SEL_SOURCES)
    get_filename_component(_SEL_FILE ${_SEL_FULLPATH} NAME)
    message(STATUS "Scanning CXX File `${PROJECT_NAME}`: ${_SEL_DIR}/${_SEL_FILE}")
endforeach()

add_executable(${PROJECT_NAME} ${SEL_HEADERS} ${SEL_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} ws2_32.lib)
endif()
