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
#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>

// build command(only Linux):
// $ g++ signaler_linux.cc -std=c++11

#define UNUSED(x) ((void)x)

int main(int argc, char* argv[]) {
  UNUSED(argc); UNUSED(argv);

  static const int RUN_COUNT = 100;
  int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

  for (int i = 0; i < RUN_COUNT; ++i) {
    char wbuf[8] = "Wakeup!";
    write(evfd, wbuf, sizeof(wbuf));

    char rbuf[8]{};
    read(evfd, rbuf, sizeof(rbuf));
    std::cout << "[" << i+1 << "] eventfd signaler read - " << rbuf << std::endl;
  }

  close(evfd);
  return 0;
}
