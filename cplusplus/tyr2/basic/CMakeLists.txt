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

set(TYR_BASIC_NAME "tyr.basic")
set(TYR_BASIC_SOURCES

  TCountdownLatch.cc
  TDate.cc
  TException.cc
  TLogStream.cc
  TLogging.cc
  TStringPiece.cc
  TThread.cc
  TThreadPool.cc
  TTimestamp.cc
  TTimezone.cc
  TUnitTest.cc
  )

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(TYR_BASIC_SOURCES ${TYR_BASIC_SOURCES}

    windows/TCurrentThreadWindows.cc
    windows/TPlatformWindows.cc
    )
else()
  set(TYR_BASIC_SOURCES ${TYR_BASIC_SOURCES} posix/TCurrentThreadPosix.cc posix/TPlatformPosix.cc)
  if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(TYR_BASIC_SOURCES ${TYR_BASIC_SOURCES}

      linux/TCurrentThreadLinux.cc
      linux/TPlatformLinux.cc
      )
  elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(TYR_BASIC_SOURCES ${TYR_BASIC_SOURCES}

      darwin/TCurrentThreadDarwin.cc
      darwin/TPlatformDarwin.cc
      )
  endif()
endif()

add_library(${TYR_BASIC_NAME} STATIC ${TYR_BASIC_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(${TYR_BASIC_NAME} winmm.lib Dbghelp.lib)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(${TYR_BASIC_NAME} pthread rt)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  target_link_libraries(${TYR_BASIC_NAME} pthread c)
endif()
