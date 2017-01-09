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

set(TYR_NET_NAME "tyr.net")
set(TYR_NET_SOURCES

  TAcceptor.cc
  TBuffer.cc
  TChannel.cc
  TConnector.cc
  TDefaultPoller.cc
  TEventLoop.cc
  TEventLoopThread.cc
  TEventLoopThreadPool.cc
  TInetAddress.cc
  TPollPoller.cc
  TPoller.cc
  TSocket.cc
  TSocketSupport.cc
  TTcpClient.cc
  TTcpConnection.cc
  TTcpServer.cc
  TTimer.cc
  TTimerQueue.cc
  )

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(TYR_NET_SOURCES ${TYR_NET_SOURCES}

    windows/TKernWrapperWindows.cc
    windows/TSocketSupportWindows.cc
    windows/TSelectPollerWindows.cc
    windows/TWakeupSignalerWindows.cc
    )
else()
  set(TYR_NET_SOURCES ${TYR_NET_SOURCES} posix/TSocketSupportPosix.cc)
  if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(TYR_NET_SOURCES ${TYR_NET_SOURCES}

      linux/TAsyncPollerLinux.cc
      linux/TSelectPollerLinux.cc
      linux/TKernWrapperLinux.cc
      )
  elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(TYR_NET_SOURCES ${TYR_NET_SOURCES}

      darwin/TAsyncPollerDarwin.cc
      darwin/TSelectPollerDarwin.cc
      darwin/TKernWrapperDarwin.cc
      )
  endif()
endif()

add_library(${TYR_NET_NAME} STATIC ${TYR_NET_SOURCES})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(${TYR_NET_NAME} ws2_32.lib)
endif()
target_link_libraries(${TYR_NET_NAME} tyr.basic)
