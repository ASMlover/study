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
#ifndef DARWIN_WAKEUPSIGNAL_H_
#define DARWIN_WAKEUPSIGNAL_H_

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

class WakeupSignal {
  int fds_[2]{}; // [0] - reader, [1] - writer

  void open(void) {
    if (0 != pipe(fds_))
      return;

    if (0 != set_nonblock(fds_[0]) || 0 != set_nonblock(fds_[1]))
      close();
  }

  void close(void) {
    if (fds_[0] > 0) {
      ::close(fds_[0]);
      fds_[0] = 0;
    }
    if (fds_[1] > 0) {
      ::close(fds_[1]);
      fds_[1] = 0;
    }
  }

  int set_nonblock(int fd) {
    int flag = fcntl(fd, F_GETFL);
    if (-1 == flag)
      return -1;
    if (0 != fcntl(fd, F_SETFL, flag | O_NONBLOCK))
      return -1;
    return 0;
  }
public:
  WakeupSignal(void) {
    open();
  }

  ~WakeupSignal(void) {
    close();
  }

  int get_reader_fd(void) const {
    return fds_[0];
  }

  bool is_opened(void) const {
    return fds_[0] > 0 && fds_[1] > 0;
  }

  int set_signal(const char* buf, size_t len) {
    return write(fds_[1], buf, len);
  }

  int get_signal(size_t len, char* buf) {
    return read(fds_[0], buf, len);
  }
};

#endif // DARWIN_WAKEUPSIGNAL_H_
