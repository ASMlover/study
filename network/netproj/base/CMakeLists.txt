# Copyright (c) 2017 ASMlover. All rights reserved.
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

set(OUTPUT_NAME "base")

aux_source_directory(. LIBBASE_SOURCES)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  aux_source_directory(./windows LIBBASE_WINDOWS_SOURCES)
  set(LIBBASE_SOURCES ${LIBBASE_SOURCES} ${LIBBASE_WINDOWS_SOURCES})
else()
  aux_source_directory(./posix LIBBASE_POSIX_SOURCES)
  set(LIBBASE_SOURCES ${LIBBASE_SOURCES} ${LIBBASE_POSIX_SOURCES})
endif()
message(STATUS "[netproj.base] LIBBASE_SOURCES: ${LIBBASE_SOURCES}")

add_library(${OUTPUT_NAME} STATIC ${LIBBASE_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(${OUTPUT_NAME} ws2_32 chaos)
else()
  target_link_libraries(${OUTPUT_NAME} chaos pthread)
endif()
