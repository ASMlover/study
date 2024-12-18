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
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include "../error.h"
#include "../primitive.h"

namespace netpp {

void startup(void) {}
void cleanup(void) {}

namespace socket {
  int close(socket_t sockfd, std::error_code& ec) {
    int r{};
    if (sockfd != kInvalidSocket) {
      clear_last_errno();
      r = error_wrapper(::close(sockfd), ec);
    }

    if (r == 0)
      ec = std::error_code();
    return r;
  }

  int read(socket_t sockfd, std::size_t len, void* buf, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    clear_last_errno();
    auto nread = error_wrapper(::read(sockfd, buf, len), ec);
    if (nread >= 0)
      ec = std::error_code();
    return nread;
  }

  int write(socket_t sockfd,
      const void* buf, std::size_t len, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    clear_last_errno();
    auto nwrote = error_wrapper(::write(sockfd, buf, len), ec);
    if (nwrote >= 0)
      ec = std::error_code();
    return nwrote;
  }

  int read_from(socket_t sockfd, std::size_t len,
      void* buf, void* addr, std::error_code& ec, bool with_v6) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    socklen_t addrlen{};
    if (with_v6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    auto nread = error_wrapper(::recvfrom(sockfd,
          buf, len, 0, static_cast<struct sockaddr*>(addr), &addrlen), ec);
    if (nread >= 0)
      ec = std::error_code();
    return nread;
  }

  int write_to(socket_t sockfd,
      const void* buf, std::size_t len, const void* addr, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    auto* to_addr = static_cast<const struct sockaddr*>(addr);
    socklen_t addrlen{};
    if (to_addr->sa_family == AF_INET6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    auto nwrote = error_wrapper(::sendto(sockfd,
          buf, len, 0, to_addr, addrlen), ec);
    if (nwrote >= 0)
      ec = std::error_code();
    return nwrote;
  }

  bool set_non_blocking(socket_t sockfd, bool mode, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return false;
    }

    clear_last_errno();
    int r = error_wrapper(::fcntl(sockfd, F_GETFL, 0), ec);
    if (r >= 0) {
      clear_last_errno();
      int flags = (mode ? (r | O_NONBLOCK) : (r & ~O_NONBLOCK));
      r = error_wrapper(::fcntl(sockfd, F_SETFL, flags), ec);
    }

    if (r >= 0) {
      ec = std::error_code();
      return true;
    }
    return false;
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
