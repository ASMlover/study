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

set(OUTPUT_NAME "_mevo.tadpole")

file(GLOB_RECURSE _MEVO_TADPOLE_HEADERS *.hh)
file(GLOB_RECURSE _MEVO_TADPOLE_SOURCES *.cc)
source_group("Header Files" FILES ${_MEVO_TADPOLE_HEADERS})
source_group("Source Files" FILES ${_MEVO_TADPOLE_SOURCES})

message(STATUS "Scanning `${PROJECT_NAME}.${OUTPUT_NAME}`:")
foreach(_MEVO_TADPOLE_FILE IN LISTS _MEVO_TADPOLE_HEADERS _MEVO_TADPOLE_SOURCES)
  get_filename_component(CURRDIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  get_filename_component(CURRFILE ${_MEVO_TADPOLE_FILE} NAME)
  message(STATUS "Scanning CXX File `${PROJECT_NAME}.${OUTPUT_NAME}`: ${CURRDIR}/${CURRFILE}")
endforeach()

add_executable(${OUTPUT_NAME} ${_MEVO_TADPOLE_HEADERS} ${_MEVO_TADPOLE_SOURCES})
target_link_libraries(${OUTPUT_NAME} _mevo)
