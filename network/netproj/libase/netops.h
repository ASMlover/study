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

#include <cstddef>
#include <cstdint>
#include <string>
#include <Chaos/Base/Platform.h>
#if defined(CHAOS_WINDOWS)
# include <WS2tcpip.h>
# if !defined(_WINDOWS_)
#   include <WinSock2.h>
# endif
  using socklen_t = int;
  using sa_family_t = int;
  using in_addr_t = int;
  using ssize_t = SSIZE_T;
  using pollfd = WSAPOLLFD;
#else
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/time.h>
# include <netdb.h>
# include <poll.h>
#endif

namespace net {

void startup(void);
void cleanup(void);

namespace socket {
  constexpr int SHUT_READ = 0;
  constexpr int SHUT_WRITE = 1;
  constexpr int SHUT_BOTH = 2;

  int open(sa_family_t family, int socket_type, int protocal);
  int close(int sockfd);
  int shutdown(int sockfd, int how);
  int bind(int sockfd, const void* addr, bool v6 = false);
  int listen(int sockfd);
  int accept(int sockfd, void* addr, bool v6 = false);
  int connect(int sockfd, const void* addr, bool v6 = false);
  ssize_t read(int sockfd, std::size_t len, void* buf);
  ssize_t write(int sockfd, const void* buf, std::size_t len);
  ssize_t readfrom(int sockfd,
      std::size_t len, void* buf, void* addr, bool v6 = false);
  ssize_t writeto(int sockfd,
      const void* buf, std::size_t len, const void* addr, bool v6 = false);
  int set_option(int sockfd,
      int level, int optname, const void* optval, socklen_t optlen);
  int get_option(int sockfd,
      int level, int optname, void* optval, socklen_t* optlen);
  void set_block(int sockfd);
  void set_nonblock(int sockfd);
}

namespace io {
  int select(int nfds, fd_set* readfds, fd_set* writefds,
      fd_set* exceptfds, const struct timeval* timeout);
  int poll(pollfd fds[], std::uint32_t nfds, int timeout);
}

namespace addr {
  std::string as_string(const void* addr, bool v6 = false);
}

}
