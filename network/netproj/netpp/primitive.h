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
#pragma once

#include <system_error>
#include "netpp_types.h"

struct sockaddr;

namespace netpp {

void startup(void);
void cleanup(void);

namespace socket {
  static constexpr int SHUT_READ = 0;
  static constexpr int SHUT_WRITE = 1;
  static constexpr int SHUT_BOTH = 2;

  socket_t open(int family, int socket_type, int protocol, std::error_code& ec);
  int close(socket_t sockfd, std::error_code& ec);
  int shutdown(socket_t sockfd, int how, std::error_code& ec);
  int bind(socket_t sockfd, const void* addr, std::error_code& ec);
  int listen(socket_t sockfd, std::error_code& ec);
  socket_t accept(
      socket_t sockfd, void* addr, std::error_code& ec, bool with_v6 = false);
  int connect(socket_t sockfd, const void* addr, std::error_code& ec);
  int read(socket_t sockfd, std::size_t len, void* buf, std::error_code& ec);
  int write(socket_t sockfd,
      const void* buf, std::size_t len, std::error_code& ec);
  int read_from(socket_t sockfd, std::size_t len,
      void* buf, void* addr, std::error_code& ec, bool with_v6 = false);
  int write_to(socket_t sockfd,
      const void* buf, std::size_t len, const void* addr, std::error_code& ec);
  bool set_non_blocking(socket_t sockfd, bool mode, std::error_code& ec);
  int set_option(socket_t sockfd, int level, int optname,
      const void* optval, std::size_t optlen, std::error_code& ec);
}

void clear_last_errno(void);
int get_errno(void);

template <typename ReturnType>
inline ReturnType error_wrapper(ReturnType r, std::error_code& ec) {
  ec = std::error_code(get_errno(), std::system_category());
  return r;
}


const char* inet_ntop(int family, const void* addr, int len, char* buf);
int inet_pton(int family, const char* buf, void* addr);

struct PollFd {
  socket_t fd{};
  short events{};
  short revents{};

  PollFd(socket_t f, short ev, short rev)
    : fd(f), events(ev), revents(rev) {
  }
};
int poll(PollFd* fds, int nfds, int timeout);

}
