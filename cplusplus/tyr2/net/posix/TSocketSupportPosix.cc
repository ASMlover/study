// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <fcntl.h>
#include <unistd.h>
#include "../unexposed/TSocketSupportUnexposed.h"
#include "../TSocketSupport.h"

namespace tyr { namespace net {
namespace SocketSupport {
  namespace unexposed {
    void kern_set_nonblock(int sockfd) {
      // non-block
      int flags = fcntl(sockfd, F_GETFL, 0);
      flags |= O_NONBLOCK;
      fcntl(sockfd, F_SETFL, flags);

      // close-on-exec
      flags = fcntl(sockfd, F_GETFD, 0);
      flags |= FD_CLOEXEC;
      fcntl(sockfd, F_SETFD, flags);
    }
  }

  void kern_set_iovec(KernIovec* vec, char* buf, size_t len) {
    vec->iov_base = buf;
    vec->iov_len = len;
  }

  ssize_t kern_read(int sockfd, void* buf, size_t len) {
    return read(sockfd, buf, len);
  }

  ssize_t kern_readv(int sockfd, const KernIovec* iov, int iovcnt) {
    return readv(sockfd, iov, iovcnt);
  }

  ssize_t kern_write(int sockfd, const void* buf, size_t len) {
    return write(sockfd, buf, len);
  }
}

}}
