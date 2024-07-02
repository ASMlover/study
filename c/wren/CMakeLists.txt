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

cmake_minimum_required(VERSION 3.14)
project(wren LANGUAGES C)

message(STATUS "Configuration: `${PROJECT_NAME}`")

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()
message(STATUS "`${PROJECT_NAME}` CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	add_definitions(
		-Wno-array-bounds
		-Wno-implicit-function-declaration
		-Wno-maybe-uninitialized
		-Wno-pedantic
		-Wno-stringop-overflow
		-Wno-unused-function
	)

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall -std=c11")
	set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -O0")
	set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3")
else()
	add_definitions(
		/DUNICODE
		/D_UNICODE
		/D_CRT_SECURE_NO_WARNINGS
		/D_CRT_NONSTDC_NO_WARNINGS
	)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /GF /GS /Gs /Zi")
	set(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MDd /Od")
	set(CMAKE_C_FLAGS_RELEASE "/DNDEBUG /MD /Ox")
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "`${PROJECT_NAME}` CMAKE_C_FLAGS: ${CMAKE_C_FLAGS_${BUILD_TYPE}}")

file(GLOB WREN_SOURCES
	main.c
	compiler.c
	core.c
	debug.c
	opt_meta.c
	opt_random.c
	primitive.c
	utils.c
	value.c
	vm.c
	io.c
	os.c
)
message(STATUS "`${PROJECT_NAME}` WREN_SOURCES: ${WREN_SOURCES}")

add_executable(${PROJECT_NAME} ${WREN_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	target_link_libraries(${PROJECT_NAME} m)
endif()
