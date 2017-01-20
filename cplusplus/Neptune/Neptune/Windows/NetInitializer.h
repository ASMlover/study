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
#ifndef NEPTUNE_WINDOWS_NETINITIALIZER_H
#define NEPTUNE_WINDOWS_NETINITIALIZER_H

#if !defined(_WINDOWS_)
# include <WinSock2.h>
#endif
#include <cstdint>
#include <Chaos/UnCopyable.h>
#include <Chaos/Logging/Logging.h>

namespace Neptune {

template <int Major = 2, int Minor = 2>
class NetInitializer : private Chaos::UnCopyable {
  long init_count_{};

  void startup(std::int8_t major, std::int8_t minor) {
    if (InterlockedIncrement(&init_count_) == 1L) {
      WSADATA wd;
      int r = WSAStartup(MAKEWORD(major, minor), &wd);

      if (r != 0)
        CHAOSLOG_SYSFATAL << "NetInitializer::startup - WinSock2 library startup failed";
    }
  }

  void cleanup(void) {
    if (InterlockedDecrement(&init_count_) == 0L)
      WSACleanup();
  }
public:
  NetInitializer(void) {
    startup(Major, Minor);
  }

  ~NetInitializer(void) {
    cleanup();
  }
};

}

#endif // NEPTUNE_WINDOWS_NETINITIALIZER_H
