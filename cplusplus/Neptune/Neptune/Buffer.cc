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
#include <cerrno>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/Buffer.h>

namespace Neptune {

const char* Buffer::kCRLF = "\r\n";

int Buffer::read_sockfd(int sockfd, int& saved_errno) {
  char extra_buf[65535];
  const std::size_t writable = writable_bytes();

  NetOps::Iovec_t vec[2];
  NetOps::socket::set_iovec(vec[0], begin() + windex_, writable);
  NetOps::socket::set_iovec(vec[1], extra_buf, sizeof(extra_buf));

  const int niov = writable < sizeof(extra_buf) ? 2 : 1;
  const int n = NetOps::socket::readv(sockfd, niov, vec);
  if (n < 0) {
    saved_errno = errno;
  }
  else if (Chaos::implicit_cast<std::size_t>(n) <= writable) {
    windex_ += n;
  }
  else {
    windex_ = buff_.size();
    append(extra_buf, n - writable);
  }

  return n;
}

}
