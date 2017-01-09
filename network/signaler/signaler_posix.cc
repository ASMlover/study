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
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

// build command(Linux|Darwin):
// $ g++ signaler_posix.cc -std=c++11

#define UNUSED(x) ((void)x)

class Wakeup {
  int fds_[2]{}; // [0] -> reader [1] -> writer
public:
  bool is_opened(void) const {
    return fds_[0] > 0 && fds_[1] > 0;
  }

  bool open(void) {
    assert(!is_opened());

    if (-1 == pipe(fds_))
      return false;
    if (-1 == set_nonblock(fds_[0]) || -1 == set_nonblock(fds_[1])) {
      close();
      return false;
    }

    return true;
  }

  void close(void) {
    ::close(fds_[0]);
    ::close(fds_[1]);
  }

  int read(char* buf, size_t len) {
    return ::read(fds_[0], buf, len);
  }

  int write(const char* buf, size_t len) {
    return ::write(fds_[1], buf, len);
  }
private:
  int set_nonblock(int fd) {
    int flag = fcntl(fd, F_GETFL);
    if (-1 == flag)
      return -1;
    if (-1 == fcntl(fd, F_SETFL, flag | O_NONBLOCK))
      return -1;

    return 0;
  }
};

int main(int argc, char* argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  static const int RUN_COUNT = 100;
  Wakeup wake;
  wake.open();

  for (int i = 0; i < RUN_COUNT; ++i) {
    char wbuf[8] = "Wakeup!";
    wake.write(wbuf, sizeof(wbuf));

    char rbuf[8]{};
    wake.read(rbuf, sizeof(rbuf));
    std::cout << "[" << i+1 << "] recv signal data - " << rbuf << std::endl;
  }

  wake.close();

  return 0;
}
