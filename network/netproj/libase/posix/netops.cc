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
#include <fcntl.h>
#include <unistd.h>
#include "../netops.h"

namespace net {

void startup(void) {}
void cleanup(void) {}

namespace socket {
  int close(int sockfd) {
    return ::close(sockfd);
  }

  ssize_t read(int sockfd, std::size_t len, void* buf) {
    return ::read(sockfd, buf, len);
  }

  ssize_t write(int sockfd, const void* buf, std::size_t len) {
    return ::write(sockfd, buf, len);
  }

  ssize_t readfrom(int sockfd,
      std::size_t len, void* buf, void* addr, bool v6) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (v6)
      addrlen = sizeof(struct sockaddr_in6);
    return ::recvfrom(sockfd, buf, len, 0, (struct sockaddr*)addr, &addrlen);
  }

  ssize_t writeto(int sockfd,
      const void* buf, std::size_t len, const void* addr, bool v6) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (v6)
      addrlen = sizeof(struct sockaddr_in6);
    return ::sendto(sockfd, buf, len, 0, (const struct sockaddr*)addr, addrlen);
  }

  int set_option(int sockfd,
      int level, int optname, const void* optval, socklen_t optlen) {
    return ::setsockopt(sockfd, level, optname, optval, optlen);
  }

  int get_option(int sockfd,
      int level, int optname, void* optval, socklen_t* optlen) {
    return ::getsockopt(sockfd, level, optname, optval, optlen);
  }

  void set_block(int sockfd) {
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    ::fcntl(sockfd, F_SETFL, flags);
  }

  void set_nonblock(int sockfd) {
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    ::fcntl(sockfd, F_SETFL, flags);
  }
}

namespace io {
  int poll(pollfd fds[], std::uint32_t nfds, int timeout) {
    return ::poll(fds, (nfds_t)nfds, timeout);
  }
}

}
