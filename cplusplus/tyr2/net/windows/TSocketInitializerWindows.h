// Copyright (c) 2017 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifndef __TYR_NET_WINDOWS_SOCKETINIIALIZERWINDOWS_HEADER_H__
#define __TYR_NET_WINDOWS_SOCKETINIIALIZERWINDOWS_HEADER_H__

#include <WinSock2.h>
#include "../../basic/TUnCopyable.h"
#include "../../basic/TLogging.h"

namespace tyr { namespace net {

template <int Major = 2, int Minor = 2>
class SocketInitializer : private basic::UnCopyable {
  long init_count_{};

  void startup(unsigned char major, unsigned char minor) {
    if (InterlockedIncrement(&init_count_) == 1) {
      WSADATA wd;
      int r = WSAStartup(MAKEWORD(major, minor), &wd);

      if (r != 0)
        TYRLOG_SYSFATAL << "SocketInitializer::startup - WinSock2 startup failed";
    }
  }

  void cleanup(void) {
    if (InterlockedDecrement(&init_count_) == 0)
      WSACleanup();
  }
public:
  SocketInitializer(void) {
    startup(Major, Minor);
  }

  ~SocketInitializer(void) {
    cleanup();
  }
};

}}

#endif // __TYR_NET_WINDOWS_SOCKETINIIALIZERWINDOWS_HEADER_H__
