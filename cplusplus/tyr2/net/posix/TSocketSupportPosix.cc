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
#include <sys/socket.h>
#include "../../basic/TLogging.h"
#include "../TEndian.h"
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

  int kern_shutdown(int sockfd) {
    int rc = shutdown(sockfd, SHUT_WR);
    if (rc < 0)
      TL_SYSERR << "SocketSupport::kern_shutdown failed";
    return 0;
  }

  int kern_close(int sockfd) {
    int rc = close(sockfd);
    if (rc < 0)
      TL_SYSERR << "SocketSupport::kern_close failed";
    return 0;
  }

  void kern_to_ip(char* buf, size_t len, const struct sockaddr* addr) {
    if (addr->sa_family == AF_INET) {
      const struct sockaddr_in* addr4 = kern_sockaddr_in_cast(addr);
      inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(len));
    }
    else if (addr->sa_family == AF_INET6) {
      const struct sockaddr_in6* addr6 = kern_sockaddr_in6_cast(addr);
      inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(len));
    }
  }

  void kern_from_ip_port(const char* ip, uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = host_to_net16(port);
    int rc = inet_pton(AF_INET, ip, &addr->sin_addr);
    if (rc <= 0)
      TL_SYSERR << "SocketSupport::kern_from_ip_port(ipv4) failed";
  }

  void kern_from_ip_port(const char* ip, uint16_t port, struct sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = host_to_net16(port);
    int rc = inet_pton(AF_INET6, ip, &addr->sin6_addr);
    if (rc <= 0)
      TL_SYSERR << "SocketSupport::kern_from_ip_port(ipv6) failed";
  }

  int kern_poll(KernPollfd fds[], uint32_t nfds, int timeout) {
    return poll(fds, (nfds_t)nfds, timeout);
  }
}

}}
