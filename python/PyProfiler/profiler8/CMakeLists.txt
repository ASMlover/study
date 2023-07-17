# Copyright (c) 2023 ASMlover. All rights reserved.
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
project(cpprof LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type [Debug|Release|RelWithDebInfo|MinSizeRel]" FORCE)
endif()
message(STATUS "`${PROJECT_NAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

add_compile_options(
	-DTRACY_ENABLE
	-DTRACY_CALLSTACK
	-DTRACY_ON_DEMAND
	-DTRACY_DELAYED_INIT
	-DTRACY_MANUAL_LIFETIME
	-DTRACY_NO_BROADCAST)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_definitions(
		/DUNICODE
		/D_UNICODE
		/D_CRT_SECURE_NO_WARNINGS
		/D_CRT_NONSTDC_NO_WARNINGS
		/wd4244
		/wd4996)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /GF /GS /Gs /Zi /EHsc /std:c++latest /await")
	set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MD /Ox")
else()
	add_definitions(
		-Wno-unused-but-set-variable
		-Wno-maybe-uninitialized)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -std=c++2a -Wall -Werror -march=native")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "`${PROJECT_NAME}` CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")

find_package(Python3 COMPONENTS Interpreter Development)
if (Python3_FOUND AND Python3_Interpreter_FOUND)
	set(PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
	set(PYTHON_LIBRARY_DIRS ${Python3_LIBRARY_DIRS})
	set(PYTHON_LIBRARIES ${Python3_LIBRARIES})
else()
	message(FATAL_ERROR "Python3 Development is required !!!")
	return()
endif()
message(STATUS "`${PROJECT_NAME}` PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")
message(STATUS "`${PROJECT_NAME}` PYTHON_LIBRARY_DIRS: ${PYTHON_LIBRARY_DIRS}")
message(STATUS "`${PROJECT_NAME}` PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}")

include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARY_DIRS})

file(GLOB CPPROF_SRC *.hh *.cc tracy/TracyClient.cpp)

add_library(${PROJECT_NAME} SHARED ${CPPROF_SRC})
target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".pyd")
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".so")
	target_link_libraries(${PROJECT_NAME} pthread dl)
endif()
