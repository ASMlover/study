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
#include <WinSock2.h>
#include <assert.h>
#include <iostream>
#include <thread>

// build command:
// \> cl.exe /Fe"a.exe" main.cc /EHsc ws2_32.lib

#define UNUSED(x) ((void)x)

template <int Major = 2, int Minor = 2>
class SocketInitializer {
  long init_count_{};

  static int startup(long& init_count, unsigned char major, unsigned char minor) {
    if (InterlockedIncrement(&init_count) == 1) {
      WSADATA wd;
      return WSAStartup(MAKEWORD(major, minor), &wd);
    }
    return -1;
  }

  static void cleanup(long& init_count) {
    if (InterlockedDecrement(&init_count) == 0)
      WSACleanup();
  }
public:
  SocketInitializer(void) {
    int r = startup(init_count_, Major, Minor);
    assert(r == 0); UNUSED(r);
  }

  ~SocketInitializer(void) {
    cleanup(init_count_);
  }
};

static void thread_callback(void) {
  SocketInitializer<> init;
  int s = socket(AF_INET, SOCK_STREAM, 0);
  assert(s > 0);

  std::cout << "s=" << s << std::endl;
}

int main(int argc, char* argv[]) {
  UNUSED(argc), UNUSED(argv);

  std::thread t1(thread_callback);
  std::thread t2(thread_callback);
  t1.join();
  t2.join();

  return 0;
}
