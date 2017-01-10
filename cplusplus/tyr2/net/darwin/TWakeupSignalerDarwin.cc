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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "../../basic/TLogging.h"
#include "TWakeupSignalerDarwin.h"

namespace tyr { namespace net {

WakeupSignaler::WakeupSignaler(void) {
  open_signaler();
  if (!is_opened())
    TYRLOG_SYSFATAL << "WakeupSignaler::WakeupSignaler - open_signaler failed";
}

WakeupSignaler::~WakeupSignaler(void) {
  close_signaler();
}

int WakeupSignaler::set_signal(const void* buf, size_t len) {
  return write(fds_[1], buf, len);
}

int WakeupSignaler::get_signal(void* buf, size_t len) {
  return read(fds_[0], buf, len);
}

void WakeupSignaler::open_signaler(void) {
  if (0 != pipe(fds_))
    TYRLOG_SYSFATAL << "WakeupSignaler::open_signaler - create pipe failed, errno=" << errno;

  set_nonblock(fds_[0]);
  set_nonblock(fds_[1]);
}

void WakeupSignaler::close_signaler(void) {
  if (fds_[0] > 0) {
    close(fds_[0]);
    fds_[0] = 0;
  }
  if (fds_[1] > 0) {
    close(fds_[1]);
    fds_[1] = 0;
  }
}

void WakeupSignaler::set_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);

  flags = fcntl(fd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  fcntl(fd, F_SETFD, flags);
}

}}
