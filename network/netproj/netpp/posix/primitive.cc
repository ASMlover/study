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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include "../primitive.h"
#include "../primitive_internal.h"

namespace netpp {

void startup(void) {}
void cleanup(void) {}

namespace socket {
  int close(int sockfd, std::error_code& ec) {
    int r{};
    if (sockfd != -1) {
      clear_last_errno();
      r = error_wrapper(::close(sockfd), ec);
    }

    if (r == 0)
      ec = std::error_code();
    return r;
  }

  int read(int sockfd, int len, void* buf, std::error_code& ec) {
    clear_last_errno();
    auto nread = error_wrapper(
        ::read(sockfd, buf, static_cast<std::size_t>(len)), ec);
    if (nread >= 0)
      ec = std::error_code();
    return nread;
  }

  int write(int sockfd, const void* buf, int len, std::error_code& ec) {
    clear_last_errno();
    auto nwrote = error_wrapper(
        ::write(sockfd, buf, static_cast<std::size_t>(len)), ec);
    if (nwrote >= 0)
      ec = std::error_code();
    return nwrote;
  }

  int readfrom(int sockfd,
      int len, void* buf, void* addr, std::error_code& ec, bool with_v6) {
    socklen_t addrlen{};
    if (with_v6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    auto nread = error_wrapper(
        ::recvfrom(sockfd, buf, static_cast<std::size_t>(len),
        0, static_cast<struct sockaddr*>(addr), &addrlen), ec);
    if (nread >= 0)
      ec = std::error_code();
    return nread;
  }

  int writeto(int sockfd,
      const void* buf, int len, const void* addr, std::error_code& ec) {
    auto* to_addr = static_cast<const struct sockaddr*>(addr);
    socklen_t addrlen{};
    if (to_addr->sa_family == AF_INET6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    auto nwrote = error_wrapper(::sendto(
          sockfd, buf, static_cast<std::size_t>(len), 0, to_addr, addrlen), ec);
    if (nwrote >= 0)
      ec = std::error_code();
    return nwrote;
  }
}

void clear_last_errno(void) {
  errno = 0;
}

int get_errno(void) {
  return errno;
}

const char* inet_ntop(int family, const void* addr, int len, char* buf) {
  return ::inet_ntop(family, addr, buf, static_cast<socklen_t>(len));
}

int inet_pton(int family, const char* buf, void* addr) {
  return ::inet_pton(family, buf, addr);
}

int poll(PollFd* fds, int nfds, int timeout) {
  return ::poll(reinterpret_cast<struct pollfd*>(fds),
      static_cast<nfds_t>(nfds), timeout);
}

}
