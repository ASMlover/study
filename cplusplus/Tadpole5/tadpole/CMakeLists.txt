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

set(TADPOLEC_NAME "tadpole")

file(GLOB_RECURSE TADPOLEC_SOURCES *.hh *.cc)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  source_group_by_dir(TADPOLEC_SOURCES)
endif()
message(STATUS "Scanning `${TADPOLEC_NAME}`:")
foreach(_TADPOLEC_REALPATH IN LISTS TADPOLEC_SOURCES)
  file(RELATIVE_PATH _TADPOLEC_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${_TADPOLEC_REALPATH})
  message(STATUS "Scanning CXX File `${TADPOLEC_NAME}`: ${_TADPOLEC_FILE}")
endforeach()

add_executable(${TADPOLEC_NAME} ${TADPOLEC_SOURCES})
