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

set(OUTPUT_NAME "nyxnet.test")
set(OUTPUT_TEST "nyxtest")

set(NYXNETTEST_SOURCEFILES
  main.cc
  client.cc
  server.cc
  )
message(STATUS "`${PROJECT_NAME}.${OUTPUT_NAME}` NYXNETTEST_SOURCEFILES: ${NYXNETTEST_SOURCEFILES}")

set(NYXTEST_SOURCEFILES
  test.cc
  )
message(STATUS "`${PROJECT_NAME}.${OUTPUT_TEST}` NYXTEST_SOURCEFILES: ${NYXTEST_SOURCEFILES}")

add_executable(${OUTPUT_NAME} ${NYXNETTEST_SOURCEFILES})
target_link_libraries(${OUTPUT_NAME} nyxcore.static)

add_executable(${OUTPUT_TEST} ${NYXTEST_SOURCEFILES})
target_link_libraries(${OUTPUT_TEST} nyxcore.static)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
else()
  target_link_libraries(${OUTPUT_NAME} pthread)
  target_link_libraries(${OUTPUT_TEST} pthread)
endif()
