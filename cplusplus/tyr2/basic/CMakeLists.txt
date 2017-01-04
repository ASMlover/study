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

set(TYR_BASIC_SOURCES
  basic/TCountdownLatch.cc
  basic/TDate.cc
  basic/TException.cc
  basic/TLogStream.cc
  basic/TLogging.cc
  basic/TStringPiece.cc
  basic/TThread.cc
  basic/TThreadPool.cc
  basic/TTimestamp.cc
  basic/TTimezone.cc
  basic/TUnitTest.cc
  )

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(TYR_BASIC_SOURCES ${TYR_BASIC_SOURCES}
    basic/windows/TCurrentThreadWindows.cc
    basic/windows/TPlatformWindows.cc
    )
else()
  set(TYR_BASIC_SOURCES ${TYR_BASIC_SOURCES} basic/posix/TCurrentThreadPosix.cc basic/posix/TPlatformPosix.cc)
  if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(TYR_BASIC_SOURCES ${TYR_BASIC_SOURCES}
      basic/linux/TCurrentThreadLinux.cc
      basic/linux/TPlatformLinux.cc
      )
  elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(TYR_BASIC_SOURCES ${TYR_BASIC_SOURCES}
      basic/darwin/TCurrentThreadDarwin.cc
      basic/darwin/TPlatformDarwin.cc
      )
  endif()
endif()
