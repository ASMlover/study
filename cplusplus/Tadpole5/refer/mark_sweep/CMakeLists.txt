# Copyright (c) 2021 ASMlover. All rights reserved.
#
#  ______             __                  ___           ____    ____
# /\__  _\           /\ \                /\_ \         /\  _`\ /\  _`\
# \/_/\ \/    __     \_\ \  _____     ___\//\ \      __\ \ \L\_\ \ \/\_\
#    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\ \ \L_L\ \ \/_/_
#     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/\ \ \/, \ \ \L\ \
#      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\\ \____/\ \____/
#       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/ \/___/  \/___/
#                             \ \_\
#                              \/_/
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

set(GC_MARKSWEEP "gc_marksweep")

file(GLOB_RECURSE GC_MARKSWEEP_SOURCES *.hh *.cc)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  source_group_by_dir(GC_MARKSWEEP_SOURCES)
endif()
message(STATUS "Scanning `${GC_MARKSWEEP}`:")
foreach(_GC_MARKSWEEP_REALPATH IN LISTS GC_MARKSWEEP_SOURCES)
  file(RELATIVE_PATH _GC_MARKSWEEP_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${_GC_MARKSWEEP_REALPATH})
  message(STATUS "Scanning CXX File `${GC_MARKSWEEP}`: ${_GC_MARKSWEEP_FILE}")
endforeach()

add_executable(${GC_MARKSWEEP} ${GC_MARKSWEEP_SOURCES})
