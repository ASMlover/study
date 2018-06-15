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
project(_nyxcore CXX)

message(STATUS "Configuration for ${PROJECT_NAME}")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "`${PROJECT_NAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(_NYXCORE_SERVER "comiling shared library for server ..." OFF)
option(_NYXCORE_ENABLE_TEST "Enable/Disable compiling test ..." OFF)
option(_NYXCORE_ENABLE_PYD "Eanble/Disable generate PYD for win32 ..." OFF)
option(_NYXCORE_ENABLE_RAW "Enable/Disable raw Python API bind ..." OFF)
option(_NYXCORE_ENABLE_WATCHER "Enable/Disable watcher ..." OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if (_NYXCORE_ENABLE_TEST)
    add_compile_options(/D_NYXCORE_ENABLE_TEST)
  endif()
  if (_NYXCORE_ENABLE_RAW)
    add_compile_options(/D_NYXCORE_ENABLE_RAW)
  endif()
  if (_NYXCORE_ENABLE_WATCHER)
    add_compile_options(/D_NYXCORE_ENABLE_WATCHER)
  endif()

  add_definitions(
    /DUNICODE
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
    /DBOOST_PYTHON_STATIC_LIB
    /D_WIN32_WINNT=0x0501
    /wd4244)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /GF /GS /Gs /Zi /EHsc")
  set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MD /Ox")

  include_directories(
    "C:/Python27amd64/include"
    "D:/Tools/boost_1_64_0"
    "D:/Tools/protobuf/src")
  link_directories(
    "C:/Python27amd64/libs"
    "D:/Tools/boost_1_64_0/stage/x64/lib"
    "D:/Tools/protobuf/cmake/cmake-build")
else()
  if (_NYXCORE_ENABLE_TEST)
    add_compile_options(-D_NYXCORE_ENABLE_TEST)
  endif()
  if (_NYXCORE_ENABLE_RAW)
    add_compile_options(-D_NYXCORE_ENABLE_RAW)
  endif()
  if (_NYXCORE_ENABLE_WATCHER)
    add_compile_options(-D_NYXCORE_ENABLE_WATCHER)
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-strict-aliasing -Wno-write-strings -std=c++1y -march=native")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")

  find_package(PythonLibs 2.7 REQUIRED)
  include_directories(
    ${PYTHON_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/../../../../../OpenLib/boost_1_63_0
    ${PROJECT_SOURCE_DIR}/../../../../../OpenLib/protobuf/src)
  link_directories(
    ${PYTHON_LIBRARIES}/../
    ${PROJECT_SOURCE_DIR}/../../../../../OpenLib/boost_1_63_0/stage/lib
    ${PROJECT_SOURCE_DIR}/../../../../../OpenLib/protobuf/cmake/cmake-build)
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "`${PROJECT_NAME}` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")

file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE SOURCE_FILES *.cc)
source_group("Headers" FILES ${HEADER_FILES})
source_group("Sources" FILES ${SOURCE_FILES})
message(STATUS "`${PROJECT_NAME}` HEADER_FILES: ${HEADER_FILES}")
message(STATUS "`${PROJECT_NAME}` SOURCE_FILES: ${SOURCE_FILES}")

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(${PROJECT_NAME}
    debug libboost_python-vc141-mt-gd-1_64 optimized libboost_python-vc141-mt-1_64
    debug python27 optimized python27
    debug libprotobufd optimized libprotobuf)
else()
  set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
  target_link_libraries(${PROJECT_NAME} pthread boost_python boost_system protobuf)
  if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME} python2.7)
  else()
    target_link_libraries(${PROJECT_NAME} python)
  endif()
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  if (_NYXCORE_ENABLE_PYD)
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
