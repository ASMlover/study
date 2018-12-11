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

set(OUTPUT_NAME "_nyxcore")
set(OUTPUT_NAME_STATIC "nyxcore.static")

file(GLOB_RECURSE NYXNETCORE_HEADERFILES *.h)
file(GLOB_RECURSE NYXNETCORE_SOURCEFILES *.cc)
source_group("Header Files" FILES NYXNETCORE_HEADERFILES)
source_group("Source Files" FILES NYXNETCORE_SOURCEFILES)
message(STATUS "`${PROJECT_NAME}.${OUTPUT_NAME}` NYXNETCORE_SOURCEFILES: ${NYXNETCORE_SOURCEFILES}")

add_library(${OUTPUT_NAME} SHARED ${NYXNETCORE_SOURCEFILES} ${NYXNETCORE_HEADERFILES})
add_library(${OUTPUT_NAME_STATIC} STATIC ${NYXNETCORE_SOURCEFILES} ${NYXNETCORE_HEADERFILES})

set_target_properties(${OUTPUT_NAME} PROPERTIES PREFIX "")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set_target_properties(${OUTPUT_NAME} PROPERTIES SUFFIX ".pyd")
  target_link_libraries(${OUTPUT_NAME} winmm.lib Dbghelp.lib)
  target_link_libraries(${OUTPUT_NAME_STATIC} winmm.lib Dbghelp.lib)
else()
  set_target_properties(${OUTPUT_NAME} PROPERTIES SUFFIX ".so")
  target_link_libraries(${OUTPUT_NAME} pthread boost_system)
  target_link_libraries(${OUTPUT_NAME_STATIC} pthread boost_system)
endif()
