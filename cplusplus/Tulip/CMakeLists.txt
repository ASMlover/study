# Copyright (c) 2018 ASMlover. All rights reserved.
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
project(Tulip CXX)

message(STATUS "Configuration for ${PROJECT_NAME}")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "${PROJECT_NAME} CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(TULIP_DEBUG_MODE "Enable/Disable debug compiling ..." OFF)
option(TULIP_ENABLE_PYD "Enable/Disable generate PYD for win32 ..." OFF)
option(TULIP_ENABLE_RAWBIND "Enable/Disable raw bind for python" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if (TULIP_DEBUG_MODE)
    add_compile_options(/DTULIP_DEBUG_MODE)
  endif()
  if (TULIP_ENABLE_RAWBIND)
    add_compile_options(/DTULIP_ENABLE_RAWBIND)
  endif()

  add_definitions(
    /DUNICODE
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
    /DBOOST_PYTHON_STATIC_LIB
    /wd4244)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /GF /GS /Gs /Zi /EHsc")
  set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd")
  set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MD /Ox")

  include_directories(
    "C:/Python27amd64/include"
    "D:/Tools/boost_1_64_0")
  link_directories(
    "C:/Python27amd64/libs"
    "D:/Tools/boost_1_64_0/stage/x64/lib")
else()
  if (TULIP_DEBUG_MODE)
    add_compile_options(-DTULIP_DEBUG_MODE)
  endif()
  if (TULIP_ENABLE_RAWBIND)
    add_compile_options(-DTULIP_ENABLE_RAWBIND)
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-strict-aliasing -std=c++1y -march=native")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")

  find_package(PythonLibs 2.7 REQUIRED)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    include_directories(
      ${PYTHON_INCLUDE_DIRS}
      ${PROJECT_SOURCE_DIR}/../../../../Opensource/boost_1_63_0)
    link_directories(
      ${PYTHON_LIBRARIES}/../
      ${PROJECT_SOURCE_DIR}/../../../../Opensource/boost_1_63_0/stage/lib)
  else()
    include_directories(
      ${PYTHON_INCLUDE_DIRS}
      ${PROJECT_SOURCE_DIR}/../../../../../OpenLib/boost_1_63_0)
    link_directories(
      ${PYTHON_LIBRARIES}/../
      ${PROJECT_SOURCE_DIR}/../../../../../OpenLib/boost_1_63_0/stage/lib)
  endif()
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "${PROJECT_NAME} CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")

file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE SOURCE_FILES *.cc)
source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})
message(STATUS "${PROJECT_NAME} HEADER_FILES: ${HEADER_FILES}")
message(STATUS "${PROJECT_NAME} SOURCE_FILES: ${SOURCE_FILES}")

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(${PROJECT_NAME}
    debug libboost_python-vc141-mt-gd-1_64 optimized libboost_python-vc141-mt-1_64)
else()
  set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
  target_link_libraries(${PROJECT_NAME} pthread boost_python)
  if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME} python2.7)
  else()
    target_link_libraries(${PROJECT_NAME} python)
  endif()
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  if (TULIP_ENABLE_PYD)
    add_custom_command(TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
      "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.dll" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pyd")
  endif()
else()
  add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.so" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.so")
endif()
