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

set(TYR_NET_SOURCES
  net/TAcceptor.cc
  net/TBuffer.cc
  net/TChannel.cc
  net/TConnector.cc
  net/TDefaultPoller.cc
  net/TEventLoop.cc
  net/TEventLoopThread.cc
  net/TEventLoopThreadPool.cc
  net/TInetAddress.cc
  net/TPollPoller.cc
  net/TPoller.cc
  net/TSocket.cc
  net/TSocketSupport.cc
  net/TTcpClient.cc
  net/TTcpConnection.cc
  net/TTcpServer.cc
  net/TTimer.cc
  net/TTimerQueue.cc
  )

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(TYR_NET_SOURCES ${TYR_NET_SOURCES}
    net/windows/TKernWrapperWindows.cc
    net/windows/TSocketSupportWindows.cc
    net/windows/TSelectPollerWindows.cc
    )
else()
  set(TYR_NET_SOURCES ${TYR_NET_SOURCES} net/posix/TSocketSupportPosix.cc)
  if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(TYR_NET_SOURCES ${TYR_NET_SOURCES}
      net/linux/TAsyncPollerLinux.cc
      net/linux/TSelectPollerLinux.cc
      net/linux/TKernWrapperLinux.cc
      )
  elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(TYR_NET_SOURCES ${TYR_NET_SOURCES}
      net/darwin/TAsyncPollerDarwin.cc
      net/darwin/TSelectPollerDarwin.cc
      net/darwin/TKernWrapperDarwin.cc
      )
  endif()
endif()
